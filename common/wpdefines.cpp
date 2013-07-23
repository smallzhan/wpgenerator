//#include "wpdefines.h"

namespace WaterPrinter
{
	//magic string and version of the waterprinter
	char magic[5] = "3DWP";
	char version[5] = "V0.1";

	// for Encrypt and Decrypt.
	char n_str[] = "0xdfa9d05bd634dd638682ef4d0625d4f5b46493875ee791bb9aff1df111e14a2dc9e68b7400057f28a7c2e13328265c53a16d7899b6e0be9e02ddabb50abe413afba59868f9f23913398e9e6f5b1931164c471c9a874de9fa3a84339983702acb1c76cc065d949f0e8fd666a589dc8dc2b189f98753cdcd5483c601ccd1e5c8e1h";
	char d_str[] = "0x1cb498b0108a350e246de5370fd90c45dda79a76adb86a51933546ec497b5fc42a57160132a7b8b4729f2bf6234543fa4e371e8975ee4c7c5281c129083945f9a3d63cb4d1c2c5e3b7e049d71e7f23672c18ff398ae2ad923ab0d0fa3871f34a682e91257ae20ef8db2f546dafe16b7217602c7ad28501f03aaf0159aca7651h";
	char e_str[] = "0x11";

    int g_col = 64;
    int g_row = 32;
    int g_dot_size = 10;
}
