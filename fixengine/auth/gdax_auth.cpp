#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cryptopp/hmac.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>


#define SOH "\x01"

std::string HexToBytes(const std::string& hex) {
	 std::vector<char> bytes;

	 for (unsigned int i = 0; i < hex.length(); i += 2) {
		 std::string byteString = hex.substr(i, 2);
	   	 char byte = (char) strtol(byteString.c_str(), NULL, 16);
	    	 bytes.push_back(byte);
	  }
	  

	  std::string xxx;
	  for(unsigned int i = 0;i<bytes.size();++i)
	    xxx +=bytes[i]; 

	  return xxx;
}



int getBase64(const std::string &content, std::string &encoded){
	    using CryptoPP::StringSource;
	    using CryptoPP::Base64Encoder;
	    using CryptoPP::StringSink;
	    
	    byte buffer[1024] = {};
	    
	    for (unsigned int i = 0; i < content.length(); i++)
	    {
		buffer[i] = content[i];
	    };
	    
	   StringSource ss(buffer, content.length(), true, new Base64Encoder( new StringSink(encoded), false));
	    //StringSource ss(encoded, true, new Base64Decoder(new HexEncoder(new StringSink(encoded))));
	    
	    return 0;
	    
}


int getHmacSha256(const std::string &key, const std::string &content, std::string &digest){
	    using CryptoPP::SecByteBlock;
	    using CryptoPP::StringSource;
	    using CryptoPP::HexEncoder;
	    using CryptoPP::StringSink;
	    using CryptoPP::HMAC;
	    using CryptoPP::HashFilter;
	    using std::transform;
	    
	    SecByteBlock byteKey((const byte*)key.data(), key.size());
	    std::string mac;
	    digest.clear();
	    
	    HMAC<CryptoPP::SHA256> hmac(byteKey, byteKey.size());
	    StringSource ss1(content, true, new HashFilter(hmac, new StringSink(mac)));
	    StringSource ss2(mac, true, new HexEncoder(new StringSink(digest)));
	    transform(digest.begin(), digest.end(), digest.begin(), ::tolower);
	    
	    return 0;
	    
}

int DecodeBase64(const std::string &encoded, std::string &decoded){
	    using CryptoPP::StringSource;
	    using CryptoPP::Base64Decoder;
	    using CryptoPP::StringSink;
	   
	    StringSource ss(encoded, true,
	 		new Base64Decoder(new StringSink(decoded)) // Base64Decoder
			); // StringSource

	    return 0;
}




extern "C" void LogonMsgGen(const char *time, const char *sender_id, const char *target_id, const char *password, const char *api_secret, char *msg){       
	   std::string payload;
	   std::string signature;
           std::string secretKey_decoded;
	   std::string signature_encoded;
	   std::string messagea_64;
           std::string body;

	   
           std::string msgType = "A";
	   std::string seqNum = "1";
	   std::string time_str(time);
	   std::string secret(api_secret);
	   std::string passphrase(password);
	   std::string key(sender_id);
	   std::string target(target_id);
	
	   body = msgType + SOH + seqNum + SOH + key + SOH + target + SOH + passphrase;

	   payload = time_str + SOH + body;

	   DecodeBase64(secret, secretKey_decoded);
	   getHmacSha256(secretKey_decoded, payload, signature);
	   std::string xxx = HexToBytes(signature);
           getBase64(xxx,messagea_64);

	   strcpy(msg, (char*)messagea_64.c_str());
	   //printf("message: %s\n", msg);
	   
}

/*	
int main(int argc, const char *argv[])
{
	const char *time = "20180306-18:22:48.286";	
	//std::string msg = LogonMsgGen(time);		
	//printf("message: %s\n", msg.c_str());
	char msg[64] ;
	LogonMsgGen(time, msg);
	printf("message: %s\n", msg);
	return 0;
}
*/



