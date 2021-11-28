
/*
#pragma once
#pragma comment(lib, "cryptopp/cryptlib.lib")
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/base64.h>

namespace Cryptography {
    std::string SHA512(std::string str)
    {
        CryptoPP::SHA512 hash;
        byte digest[CryptoPP::SHA512::DIGESTSIZE];

        hash.CalculateDigest(digest, (byte*)str.c_str(), str.length());

        CryptoPP::HexEncoder encoder;

        std::string output;

        encoder.Attach(new CryptoPP::StringSink(output));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();

        return output;
    }

    std::string Base64Decode(const std::string& encoded_string)
    {
        std::string decoded;
        CryptoPP::StringSource ss(encoded_string, true,
            new CryptoPP::Base64Decoder(
                new CryptoPP::StringSink(decoded)
            ));

        return decoded;
    }

    std::string Base64Encode(const std::string& input_string)
    {
        std::string decoded;
        CryptoPP::StringSource ss(input_string, true,
            new CryptoPP::Base64Encoder(
                new CryptoPP::StringSink(decoded), false
            ));

        return decoded;
    }

    std::string DecryptData(std::string input, std::string key, std::string iv)
    {
        //VM_EAGLE_BLACK_START
        std::string output;

        try {

            CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption((byte*)key.data(), key.size(), (byte*)iv.data());

            CryptoPP::StringSource decryptor(input, true,
                new CryptoPP::Base64Decoder(
                    new CryptoPP::StreamTransformationFilter(decryption, new CryptoPP::StringSink(output), CryptoPP::BlockPaddingSchemeDef::PKCS_PADDING)
                )
            );
        }
        catch (const CryptoPP::Exception)
        {
            output = "";
        }

        //VM_EAGLE_BLACK_END

        return output;
    }

    std::string EncryptData(std::string input, std::string key, std::string iv)
    {
        //VM_EAGLE_BLACK_START
        std::string output;

        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption((byte*)key.data(), key.size(), (byte*)iv.data());
        CryptoPP::StringSource encryptor(input, true,
            new CryptoPP::StreamTransformationFilter(encryption,
                new CryptoPP::Base64Encoder(
                    new CryptoPP::StringSink(output),
                    false
                )
            )
        );
        //VM_EAGLE_BLACK_END

        return output;
    }

}
*/