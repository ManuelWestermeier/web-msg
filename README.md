# WEB MSG

# User1@Server1 wants to send "HELLO WORLD" to User2@Server2

## SEND_CLIENT_SIDE

### 1. Get User2@Server2 public data (server+client)

SEND_CLIENT_SIDE:

S_PK = fetch (https://raw.githubusercontent.com/ManuelWestermeier/web-msg-data/refs/heads/main/pk.txt)

S_IP = fetch (https://raw.githubusercontent.com/ManuelWestermeier/web-msg-data/refs/heads/main/ip.txt)

U_PK = fetch (https://raw.githubusercontent.com/{Server2}/web-msg-data/refs/heads/main/user/{User2}/pk.txt)

### 2. Validate the data

sign_check(S_PK, S_IP)

sign_check(S_PK, U_PK)

### 3. Construct the frame

SYM_K = random AES Key
SYM_IV = random AES iv

POCKET =

```c
User1@Server1 // from
User2@Server2 // to
U_PK(SYM_K+SYM_IV) // data en/decryption key
SYM_K+SYM_IV(RAW_DATA) // the encrypted data
RANDOM_ID // for the proof of work
HASH // hash of pocket with 10 zeros on the front
SIGN // sender.sk enc hash of entrie pocket
```

### 4. Store frame

MESSAGE_USER = Server1 | Some other random Github Account

fetch store (https://raw.githubusercontent.com/{MESSAGE_USER}/web-msg-messages/refs/heads/main/{POCKET.SIGN}.txt, POCKET)

=> POST http://{S_IP}/send `{Server1}|{POCKET.SIGN}`

## RECEIVE_SERVER_SIDE

### 5. validate pocket

has (`Server1 POCKET.SIGN`)

POCKET = fetch (https://raw.githubusercontent.com/{MESSAGE_USER}/web-msg-messages/refs/heads/main/{POCKET.SIGN}.txt)

U_PK = fetch (https://raw.githubusercontent.com/{Server1}/web-msg-data/refs/heads/main/user/{User1}/pk.txt)

(VALIDATE IT TOO (Server1.pk...))

VALIDATE THE PROOF OF WORK (
    HASH THE POCKET => CHECK 10 zeros on start
)

VALITADTE THE SIGN (
    HASH THE FULL POCKET => VALIDATE WITH SENDER PUBLIC KEY FROM U_PK
)

if all is good append the pocket identifyer (`{Server1}|{POCKET.SIGN}`) to the messages list on (https://raw.githubusercontent.com/{Server2}/web-msg-data/refs/heads/main/user/{User1}/messages-list.txt) if the 