#ifndef _WPRINTER_WPENCODER_H_
#define _WPRINTER_WPENCODER_H_

#include <iostream>
#include <string>
#include <cstdio>
#include "wpdefines.h"

//#define _NO_CRYPT
#ifndef _NO_CRYPT
#include "rsa.h"
using CryptoPP::RSA;

#include "integer.h"
using CryptoPP::Integer;

#include "osrng.h"
using CryptoPP::AutoSeededRandomPool;

#endif

namespace WaterPrinter
{

#define DATA_LEN 1024

	class WPEncoder
	{
	public:
		WPEncoder();
		WPEncoder(const char* data, const char* fname);
		~WPEncoder();

	
		void encodeBits();
		void decodeBits();

		void setBitmap(const std::string &bitmap);
		void setOutfile(const std::string &outfile);
		void setCryptType(bool is_crypt); 

		std::string getBitmap() const ;

	private:
		std::string outfile_;		

		bool loadFile();
		bool writeFile(void* data, size_t sz);

		void enCrypt();
		void deCrypt();

		//std::string bitmap_raw_;

		int encoded_data_[DATA_LEN];

#ifndef _NO_CRYPT
		Integer n_;
		Integer e_;
		Integer d_;

		RSA::PrivateKey priv_key_;
		RSA::PublicKey pub_key_;
		AutoSeededRandomPool prng_;
#endif
		std::string cipher_;
		std::string plain_;



		bool is_crypt_;
		//std::string bitmap_coded_;
	};
}
#endif
