# WEB MSG

## User1@Server1 wants to send "HELLO WORLD" to User2@Server2

### 1. get User2@Server2 public data (server+client)

S_PK = fetch (https://raw.githubusercontent.com/ManuelWestermeier/web-msg-data/refs/heads/main/pk.txt)

S_IP = fetch (https://raw.githubusercontent.com/ManuelWestermeier/web-msg-data/refs/heads/main/ip.txt)

U_PK = fetch (https://raw.githubusercontent.com/{Server2}/web-msg-data/refs/heads/main/user/{User2}/pk.txt)

### 2. Validate the data

sign_check(S_PK, S_IP)

sign_check(S_PK, U_PK)

### 3. Construct the frame

SYM_K = random AES Key
SYM_IV = random AES iv

```txt
User1@Server1 // from
User2@Server2 // to
U_PK(SYM_K+SYM_IV) // data en/decryption key
SYM_K+SYM_IV(RAW_DATA) // the encrypted data

```
