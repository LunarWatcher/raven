#include "SSLConfig.hpp"

#include <csignal>
#include <openssl/asn1.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <iostream>

namespace raven {
SSLConfig::SSLConfig(
    std::filesystem::path&& certPath,
    std::filesystem::path&& privateKeyPath,
    bool withDeeplyInsecureTestCertificates
) :
    certPath(std::move(certPath)),
    privateKeyPath(std::move(privateKeyPath)) {

    // For some reason, remote terminations cause a SIGPIPE that, with OSSL, causes termination
    // TODO: Does this also apply to accept() in general, and that it's just easier to provoke with handshake errors?
    signal(SIGPIPE, SIG_IGN);

    if (withDeeplyInsecureTestCertificates) {
        std::cerr << "Generating test certificates. If you're using this in production, you have just done "
            "something very fucking stupid, and you should set withDeeplyInsecureTestCertificates to false and "
            "read the documentation."
                  << std::endl;
        insecureGenerateTestCertificates();
    }

    initSSL();
}

SSLConfig::SSLConfig(SSLConfig&& other) noexcept :
    certPath(std::move(other.certPath)),
    privateKeyPath(std::move(other.privateKeyPath)),
    contextPool(std::move(other.contextPool))
{
}


SSLConfig::~SSLConfig() {
    for (auto& [_, sslCtx] : contextPool) {
        SSL_CTX_free(sslCtx);
    }
}

void SSLConfig::insecureGenerateTestCertificates() {
    if (std::filesystem::exists(privateKeyPath) && std::filesystem::exists(certPath)) {
        return;
    }
    std::filesystem::create_directories(privateKeyPath.parent_path());
    std::filesystem::create_directories(certPath.parent_path());
    auto* privateKey = EVP_RSA_gen(4096);

    auto* x509 = X509_new();

    ASN1_INTEGER_set(X509_get_serialNumber(x509), 69);

    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 60ll * 60 * 24 * 365 * 10); // 10 years from now

    X509_set_pubkey(x509, privateKey);
    auto name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(
        name,
        "C",
        MBSTRING_ASC,
        (unsigned char*) "NO", -1, -1, 0
    );
    X509_NAME_add_entry_by_txt(
        name,
        "O",
        MBSTRING_ASC,
        (unsigned char*) "codeberg:LunarWatcher/raven", -1, -1, 0
    );
    // TODO: would be nice if this also worked with IP out of the box.
    // https://stackoverflow.com/a/41366949 suggests there's a nice way to do it, but it's CLI-based rather than
    // C-based and 5 minutes failed to translate it
    X509_NAME_add_entry_by_txt(
        name,
        "CN",
        MBSTRING_ASC,
        (unsigned char*) "localhost", -1, -1, 0
    );

    X509_set_issuer_name(
        x509, name
    );

    if (!X509_sign(
            x509,
            privateKey,
            // nullptr // Null with ED25519
            EVP_sha512()
        )) {
            
        throw std::runtime_error(std::string {
                ERR_error_string(
                    ERR_get_error(), nullptr
                )
            });
    }

    // Pretty sure this is standard C: https://en.cppreference.com/w/c/io/fopen
    FILE* keyFilePtr = fopen(
        this->privateKeyPath.string().c_str(),
        "wb"
    );
    FILE* certFilePtr = fopen(
        this->certPath.string().c_str(),
        "wb"
    );

    PEM_write_PrivateKey(
        keyFilePtr,
        privateKey,
        nullptr, // encryption method (disabled)
        nullptr, // password (disabled, for obvious reasons)
        0, // password length
        nullptr, nullptr // callback shit?
    );

    PEM_write_X509(
        certFilePtr, x509
    );

    fclose(keyFilePtr);
    fclose(certFilePtr);

    EVP_PKEY_free(privateKey);

    X509_free(x509);

}

void SSLConfig::initSSL() {
    if (!std::filesystem::exists(privateKeyPath) || !std::filesystem::exists(certPath)) {
        throw std::runtime_error(
            "Private key and/or certPath does not point to a file that exists"
        );
    }
    auto sslCtx = SSL_CTX_new(
        SSLv23_server_method()
    );

    if (SSL_CTX_use_certificate_file(
            sslCtx, this->certPath.string().c_str(), SSL_FILETYPE_PEM
    ) != 1) {
        throw std::runtime_error(
            "OpenSSL failed to load cert file"
        );
    }
    if (SSL_CTX_use_PrivateKey_file(
            sslCtx, this->privateKeyPath.string().c_str(), SSL_FILETYPE_PEM
    ) != 1) {
        throw std::runtime_error(
            "OpenSSL failed to load cert file"
        );
    }

    contextPool[std::this_thread::get_id()] = sslCtx;
}

SSL_CTX* SSLConfig::getHandle() {
    // TODO: not sure if doing it this way makes sense. Could probably speed it up slightly by using a static
    // std::atomic<int> that provides a thread_local idx, and using that for lookup instead (O(slightly faster 1))
    // That said, OSSL's benchmarks say using an SSL_CTX pool is like 0.3ms faster or so (on the benchmark's hardware):
    // https://openssl-library.org/performance
    auto it = contextPool.find(std::this_thread::get_id());
    if (it == nullptr) {
        initSSL();
        return getHandle();
    }

    return it->second;
}

}
