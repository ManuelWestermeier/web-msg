WEB MSG
=======

Ein dezentrales, verschlüsseltes Nachrichtensystem auf Basis von GitHub-Repositories.

Beispiel
--------

**User1@Server1** will die Nachricht `"HELLO WORLD"` an **User2@Server2** senden.

* * *

SEND\_CLIENT\_SIDE
------------------

### 1\. Öffentliche Daten holen

    S_PK = fetch("https://raw.githubusercontent.com/ManuelWestermeier/web-msg-data/refs/heads/main/pk.txt")
    S_IP = fetch("https://raw.githubusercontent.com/ManuelWestermeier/web-msg-data/refs/heads/main/ip.txt")
    
    U_PK = fetch("https://raw.githubusercontent.com/{Server2}/web-msg-data/refs/heads/main/user/{User2}/pk.txt")
    

### 2\. Signaturen prüfen

    sign_check(S_PK, S_IP)
    sign_check(S_PK, U_PK)
    

### 3\. Nachricht konstruieren

    SYM_K = random AES key
    SYM_IV = random AES IV
    RAW_DATA = "HELLO WORLD"
    
    ENC_MSG = AES_Encrypt(SYM_K, SYM_IV, RAW_DATA)
    ENC_KEY = RSA_Encrypt(U_PK, SYM_K + SYM_IV)
    NONCE = random 64-bit value
    
    // Proof of Work: SHA256 mit 10 führenden Nullen
    while (true) {
      HASH = SHA256(POCKET_DATA)
      if (HASH.startsWith("0000000000")) break
      NONCE = new_random()
      update_nonce(NONCE)
    }
    
    SIGN = Sign(Sender1.SK, HASH)
    

**Finales POCKET (als JSON):**

    {
      "from": "User1@Server1",
      "to": "User2@Server2",
      "enc_key": "<RSA verschlüsselt>",
      "enc_msg": "<AES-verschlüsselt>",
      "nonce": "<zufällig>",
      "hash": "<SHA256>",
      "sign": "<Digitale Signatur>"
    }
    

`SIGN` dient als Nachricht-ID und Replay-Schutz.

### 4\. Nachricht speichern

    MESSAGE_USER = Server1 oder anderer GitHub-Account
    STORE_URL = "https://raw.githubusercontent.com/{MESSAGE_USER}/web-msg-messages/refs/heads/main/{SIGN}.txt"
    
    upload(STORE_URL, POCKET)
    POST("http://{S_IP}/send", body: "{Server1}|{SIGN}")
    

* * *

RECEIVER\_SERVER\_SIDE
----------------------

### 5\. Empfang & Validierung

    POCKET = fetch("https://raw.githubusercontent.com/{MESSAGE_USER}/web-msg-messages/refs/heads/main/{SIGN}.txt")
    U_PK = fetch("https://raw.githubusercontent.com/{Server1}/web-msg-data/refs/heads/main/user/{User1}/pk.txt")
    S_PK = fetch("https://raw.githubusercontent.com/{Server1}/web-msg-data/refs/heads/main/pk.txt")
    

#### Validierung:

*   **Replay-Schutz:** SIGN darf nicht in `messages-list.txt` vorkommen.
*   **Hash prüfen:** SHA256(POCKET) == hash
*   **Proof of Work:** hash beginnt mit 10 Nullen
*   **Signatur prüfen:** verify(U\_PK, SIGN, hash)

**Falls gültig:**

    append "Server1|{SIGN}" to:
    https://raw.githubusercontent.com/{Server2}/web-msg-data/refs/heads/main/user/{User1}/messages-list.txt
    

Optional: Push-Benachrichtigung senden.

* * *

RECEIVE\_CLIENT\_SIDE
---------------------

### 6\. Nachrichten abrufen und entschlüsseln

    messages = fetch(".../messages-list.txt")
    
    foreach (entry in messages) {
      [sender, SIGN] = entry.split("|")
      POCKET = fetch("https://raw.githubusercontent.com/{sender}/web-msg-messages/refs/heads/main/{SIGN}.txt")
    
      // Validieren & Entschlüsseln
      validate(POCKET)
      SYM_K, SYM_IV = RSA_Decrypt(User1.SK, POCKET.enc_key)
      RAW = AES_Decrypt(SYM_K, SYM_IV, POCKET.enc_msg)
    
      display(RAW)
    }
    

* * *

Eigenschaften
-------------

*   🔐 Ende-zu-Ende-Verschlüsselung (AES-256-GCM)
*   🔏 Digitale Signaturen zur Senderverifikation
*   🧠 Proof of Work mit SHA256(POCKET) → 10 führende Nullen
*   🧱 Replay-Schutz durch SIGN-ID
*   📦 Speicher über GitHub-Repositories

Sicherheitshinweise
-------------------

*   Alle Schlüssel und IPs müssen signiert und geprüft werden.
*   GitHub ist öffentlich → alle Inhalte müssen verschlüsselt sein.
*   Keine Zeitstempel → Reihenfolge von Nachrichten nicht garantiert.
*   GitHub kann Caching verursachen → Verzögerungen möglich.