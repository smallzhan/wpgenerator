#include <sstream>
#include <iostream>
#include "wpencoder.h"
#include <cstring>


#ifndef _NO_CRYPT
using namespace CryptoPP;
#include "hex.h"
#endif

namespace WaterPrinter
{
	static const size_t s_int_len = 32;
	static const size_t s_wp_col = 64;
	static const size_t s_max_engypht = 64; // actually, is 86 for 1024 bit.
	static const size_t s_segment_len = 128; // for 1024 bit encrypt. 

	static const char* s_dec_num[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
	static const char* s_hex_num[] = {"a", "b", "c", "d", "e", "f"};

	WPEncoder::WPEncoder()
		:plain_("")
		,outfile_("out.3dwp")
#ifndef _NO_CRYPT
		,n_(n_str)
		,e_(e_str)
		,d_(d_str)
		,is_crypt_(true)
#else
		,is_crypt_(false)
#endif
	{
#ifndef _NO_CRYPT
		priv_key_.Initialize(n_, e_, d_);
		pub_key_.Initialize(n_, e_);
#endif
	}

	WPEncoder::WPEncoder(const char* data, const char* filename) 
		: plain_(data)
		,outfile_(filename)
#ifndef _NO_CRYPT
		,n_(n_str)
		,e_(e_str)
		,d_(d_str)
		,is_crypt_(true)
#else
		,is_crypt_(false)
#endif
	{
#ifndef _NO_CRYPT
		priv_key_.Initialize(n_, e_, d_);
		pub_key_.Initialize(n_, e_);
#endif
	}

	WPEncoder::~WPEncoder() 
	{

	}

	void WPEncoder::setOutfile(const std::string& outfile)
	{
		outfile_ = outfile;
	}

	void WPEncoder::setBitmap(const std::string& bitmap)
	{
		plain_ = bitmap;
	}

	void WPEncoder::setCryptType(bool is_crypt)
	{
#ifndef _NO_CRYPT
		is_crypt_ = is_crypt;
#else
		is_crypt_ = false;
#endif
	}
	void WPEncoder::encodeBits()
	{
		enCrypt();
		const char* c_bitmap = cipher_.data();

		//fwrite(c_bitmap, sizeof(c_bitmap), out_file_);
		memset(encoded_data_, 0, DATA_LEN * sizeof(int));

		encoded_data_[0] = *((int*) magic); /* 0x33445750 3DWP */
		encoded_data_[1] = *((int*) version); /*0x56302e31 V0.1 */
		encoded_data_[2] = cipher_.size() / sizeof(int) + 1; /*col, row */

		size_t idx = 3;
		int *cur_data = (int*) c_bitmap;
		for (size_t i = 0; i < cipher_.size(); i += sizeof(int))
		{

			encoded_data_[idx++] = cur_data[0];
			cur_data += 1;
		}
		//encoded_data_[idx++] = 0x00000024;

		printf("idx = %d\n", idx);

		if (!writeFile(encoded_data_, idx))
		{
			printf("write file failed\n");
		}

		//std::cout << "encoded text: " << std::endl << cipher_ << std::endl;
	}

	bool WPEncoder::writeFile(void* data, size_t idx)
	{
		FILE *fp;
		if ((fp = fopen(outfile_.c_str(), "wb")) == NULL)
		{
			return false;
		}
		else
		{
			fwrite(data, sizeof(int), idx, fp);
		}
		if (fp)
		{
			fclose(fp);
		}

		return true;
	}

	void WPEncoder::decodeBits()
	{
		if (!loadFile())
		{
			printf("something weird happened, load file %s failed!\n", outfile_.c_str());
		}
		else
		{
			deCrypt();
		}
		//std::cout << "decoded text:" << std::endl << plain_ << std::endl;
	}

	void WPEncoder::enCrypt()
	{
		//std::cout  << raw_hex_string << std::endl;
		if (!is_crypt_)
		{
			cipher_ = plain_;
			return;
		}
#ifndef _NO_CRYPT
		size_t hex_size = plain_.size();

		const char* cur_text = plain_.data();

		RSAES_OAEP_SHA_Encryptor e(pub_key_);

		int segment = hex_size / s_max_engypht;
		if (hex_size % s_max_engypht != 0) segment += 1;

		char header[3];
		header[0] = segment;
		header[1] = 1;
		header[2] = 0;

		std::stringstream sbody;
		std::string plain;
		std::string cipher;
		sbody.str("");

		while (hex_size >= s_max_engypht)
		{
			plain = std::string(cur_text, s_max_engypht);
			StringSource ss(plain, true, new PK_EncryptorFilter(prng_, e, new StringSink(cipher)));
			hex_size -= s_max_engypht;
			cur_text += s_max_engypht;
			//std::cout << "size: " << cipher.size() << std::endl;
		}
		if (hex_size > 0)
		{
			plain = std::string(cur_text, hex_size);
			StringSource(plain, true, new PK_EncryptorFilter(prng_, e, new StringSink(cipher)));
			header[1] = hex_size;
		}

		cipher_ = std::string(header)  + cipher;//shead.str() + cipher;//sbody.str();
#endif
	}

	void WPEncoder::deCrypt()
	{


		const char *cur_cipher = (char* ) (encoded_data_ + 3);//encrypt_.data();
		if (!is_crypt_)
		{
			plain_ = std::string(cur_cipher, (encoded_data_[2] - 1) * sizeof(int));
			return;
		}
#ifndef _NO_CRYPT
		int segment = (unsigned char)cur_cipher[0];
		int remainder = (unsigned char)cur_cipher[1];

		cur_cipher += 2;


		RSAES_OAEP_SHA_Decryptor d(priv_key_);
		std::string plain, cipher;

		//int seg_len = (cipher_.size() - 2) / segment;
		//if ((cipher_.size() - 2) % segment)
		//{
		//	seg_len = (cipher_.size() - 2 - remainder) / (segment - 1);
		//}
		//seg_len = 128;
		for (size_t i = 0; i < segment; ++i)
		{
			cipher = std::string(cur_cipher, s_segment_len);
			StringSource(cipher, true, new PK_DecryptorFilter(prng_, d, new StringSink(plain)));
			cur_cipher += s_segment_len;
		}

		//cipher = std::string(cur_cipher, remainder == 1? s_segment_len: remainder);
		//StringSource(cipher, true, new PK_DecryptorFilter(prng_, d, new StringSink(plain)));

		plain_ = plain;

#endif

		//std::cout << "recovered plain text:" << std::endl << plain_ << std::endl;
	}



	bool WPEncoder::loadFile()
	{
		memset(encoded_data_, 0, DATA_LEN * sizeof(int));
		FILE *fp;
		if ((fp = fopen(outfile_.c_str(), "rb")) == NULL)
		{
			return false;
		}
		else
		{
			fread(encoded_data_, sizeof(int), 3, fp);
			if (encoded_data_[0] != *((int *)magic)/*0x33445750*/
                || encoded_data_[1] != *((int*) version)) /*0x56302e31)*/
			{
				if (fp) fclose(fp);
				printf("data header error: %d:%d, should be %d:%d \n",
					   encoded_data_[0], encoded_data_[1],
					   *((int *)magic), *((int *) version));
				return false;
			}
			else // little endian!!!
			{
				size_t idx_len = encoded_data_[2];

				fread(encoded_data_ + 3, sizeof(int), idx_len, fp);
			}
			if (fp) fclose(fp);
			return true;
		}
	}

	std::string WPEncoder::getBitmap() const
	{
		std::stringstream sstr;
		const char* pbit = plain_.data();

		for (int i = plain_.size() - 8; i >= 0; i -= 8)
		{
			sstr << std::string(pbit + i, 8);
		}

		return sstr.str();
	}

	void WPEncoder::addFile(const char* fname)
	{
		std::string first(plain_);
		size_t base_idx = outfile_.find_first_of(".");
		std::string new_file;
		if (base_idx != std::string::npos)
		{
			new_file = outfile_.substr(0, base_idx) + "_" + std::string(fname);
		}
		else
		{
			new_file = outfile_ + "_" + std::string(fname);
		}

		setOutfile(std::string(fname));
		decodeBits();

		plain_ = first + plain_;


		setOutfile(new_file);
		encodeBits();
	}
}

/************************************************
 * test
 *************************************************/

#if 0
int main(int argc, char** argv)
{
    WPEncoder encoder;
    encoder.setOutfile(argv[1]);
    encoder.setCryptType(atoi(argv[2]));
    encoder.decodeBits();

    encoder.addFile(argv[3]);
    std::string out_str = encoder.getBitmap();

    const char* p_bits = out_str.c_str();
    for (size_t i = 0; i < out_str.size(); ++i)
    {
        if (i % 8 == 0)
        {
            printf("\n");
        }
        char digit = out_str[i];
        for (int j = 7; j >= 0; --j)
        {
            if (digit & (1 << j))
                printf("#");
            else
                printf("-");
        }
    }
    printf("\n");
}
#endif
