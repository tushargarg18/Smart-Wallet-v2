# Smart-Wallet-v2
A wallet having electronics to record real time transactions with the help of voice command and saving the data on a secured server which user can securely access later on. 
It consists of GPS tracker function which enable user to track wallet remotely if the wallet is lost or to track children if required. 
It also consists of a SOS safety button which sends alert to devices if activated. 
It comprises RFID safe pockets to save your cards from data cloning.

Code records audio data at 16000Hz and encode it in base64 format, finally storing it in a SD card.
This base64 encoded string is transmitted on server.
On server side with the help of Google Speech Recognition api this audio is converted to text.
And after performing data extraction the transaction details are stored on secured server, which is accessed with the help of either mobile application or via web page. 
