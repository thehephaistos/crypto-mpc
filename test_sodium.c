// test_sodium.c
#include <stdio.h>
#include <sodium.h>

int main(void) {
    // Initialize libsodium
    if (sodium_init() < 0) {
        printf("❌ libsodium initialization failed!\n");
        return 1;
    }
    
    printf("✅ libsodium version: %s\n", sodium_version_string());
    
    // Test random number generation
    unsigned char random_bytes[32];
    randombytes_buf(random_bytes, sizeof(random_bytes));
    
    printf("✅ Random bytes generated: ");
    for (int i = 0; i < 8; i++) {
        printf("%02x", random_bytes[i]);
    }
    printf(".. .\n");
    
    // Test encryption
    unsigned char key[crypto_secretbox_KEYBYTES];
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char message[] = "Hello Aegis!";
    unsigned char ciphertext[sizeof(message) + crypto_secretbox_MACBYTES];
    
    crypto_secretbox_keygen(key);
    randombytes_buf(nonce, sizeof(nonce));
    
    crypto_secretbox_easy(ciphertext, message, sizeof(message), nonce, key);
    printf("✅ Encryption successful!\n");
    
    // Test decryption
    unsigned char decrypted[sizeof(message)];
    if (crypto_secretbox_open_easy(decrypted, ciphertext, 
                                     sizeof(ciphertext), nonce, key) != 0) {
        printf("❌ Decryption failed!\n");
        return 1;
    }
    printf("✅ Decryption successful!\n");
    printf("✅ Decrypted message:  %s\n", decrypted);
    
    printf("\n🎉 libsodium is working perfectly on your Mac!\n");
    return 0;
}