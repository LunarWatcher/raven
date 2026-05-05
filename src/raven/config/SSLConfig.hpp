#pragma once

#include <filesystem>
#include <openssl/crypto.h>
#include <thread>
#include <unordered_map>

namespace raven {

class SSLConfig {
protected:
    std::filesystem::path certPath;
    std::filesystem::path privateKeyPath;

    /**
     * Utility method for generating (insecure) certificates. Never ever use this nor the output certificates in a
     * production setting. This should only be used for testing or in local setup.
     *
     * This does not and will not expose any config options. Its goal is to provide certificates that are technically
     * usable by consuming applications, and nothing more.
     */
    void insecureGenerateTestCertificates();

    void initSSL();

    std::unordered_map<decltype(std::this_thread::get_id()), SSL_CTX*> contextPool;

public:
    /**
     * Initialises SSLConfig
     *
     * \param certPath                              The path to the certificate (must be in PEM format)
     * \param privateKeyPath                        The path to the private key (must be in PEM format)
     * \param withDeeplyInsecureTestCertificates    Whether or not to generate the certificates. This must not be set to
     *      true outside tests and local setups. The setup is not designed to be secure, only to provide convenient
     *      certificates for testing HTTPS servers without having to properly self-sign a certificate.
     *
     *      This must never be true in production (unless you'd like to invite trouble), or in general outside your
     *      local environment.
     */
    SSLConfig(
        std::filesystem::path&& certPath,
        std::filesystem::path&& privateKeyPath,
        bool withDeeplyInsecureTestCertificates = false
    );
    SSLConfig(SSLConfig&& other) noexcept;
    virtual ~SSLConfig();

    SSL_CTX* getHandle();
};

}
