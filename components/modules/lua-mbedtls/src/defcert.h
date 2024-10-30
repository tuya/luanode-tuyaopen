/* Default self-signed certificate and private key in DER format */

static unsigned char defcert[] = {
	0x30, 0x82, 0x02, 0xc2, 0x30, 0x82, 0x01, 0xaa, 0x02, 0x09, 0x00, 0xd5,
	0x8b, 0x56, 0xce, 0x10, 0x5a, 0x8c, 0xca, 0x30, 0x0d, 0x06, 0x09, 0x2a,
	0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x23,
	0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x43,
	0x41, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0b,
	0x6c, 0x75, 0x61, 0x2d, 0x6d, 0x62, 0x65, 0x64, 0x74, 0x6c, 0x73, 0x30,
	0x1e, 0x17, 0x0d, 0x32, 0x30, 0x31, 0x30, 0x32, 0x30, 0x32, 0x32, 0x30,
	0x30, 0x33, 0x36, 0x5a, 0x17, 0x0d, 0x34, 0x30, 0x31, 0x30, 0x31, 0x35,
	0x32, 0x32, 0x30, 0x30, 0x33, 0x36, 0x5a, 0x30, 0x23, 0x31, 0x0b, 0x30,
	0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x41, 0x31, 0x14,
	0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0b, 0x6c, 0x75, 0x61,
	0x2d, 0x6d, 0x62, 0x65, 0x64, 0x74, 0x6c, 0x73, 0x30, 0x82, 0x01, 0x22,
	0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
	0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a,
	0x02, 0x82, 0x01, 0x01, 0x00, 0xb9, 0xcd, 0x8b, 0x41, 0xdb, 0x05, 0xd5,
	0x6c, 0x63, 0x6e, 0xc7, 0x88, 0x8c, 0xb1, 0xa1, 0x50, 0x79, 0x27, 0xc3,
	0xda, 0x97, 0xed, 0xaf, 0xe8, 0x19, 0xf8, 0xab, 0x3f, 0x66, 0x84, 0x7f,
	0x92, 0x9e, 0x47, 0x8a, 0x8d, 0x6b, 0xa4, 0x7e, 0x54, 0x34, 0xe7, 0x1f,
	0xe1, 0xf8, 0x15, 0xcc, 0x4f, 0x61, 0xef, 0x0f, 0x0a, 0xea, 0x41, 0xe3,
	0x9f, 0x8d, 0x55, 0x85, 0xeb, 0x1f, 0xde, 0xde, 0x45, 0xc9, 0x12, 0xe1,
	0x78, 0xa7, 0x6d, 0xbb, 0x16, 0xfc, 0xd2, 0xe2, 0x53, 0xa7, 0xb2, 0x8f,
	0xa6, 0xe0, 0x32, 0xdc, 0xf0, 0x24, 0xe9, 0x82, 0xba, 0xdb, 0xce, 0xa4,
	0x31, 0x77, 0x85, 0x45, 0xea, 0x6a, 0x50, 0x87, 0x71, 0xff, 0xf4, 0x8c,
	0x28, 0x18, 0x50, 0x8d, 0x44, 0x96, 0x93, 0xb5, 0x72, 0xff, 0x47, 0x36,
	0xa2, 0x60, 0x4d, 0x2a, 0xff, 0xe1, 0xb0, 0x73, 0xa8, 0x83, 0xc4, 0xb5,
	0x82, 0xd0, 0x3a, 0x56, 0x05, 0x68, 0x57, 0xd4, 0x22, 0x12, 0x42, 0x7b,
	0xab, 0x65, 0xf3, 0x1b, 0xc4, 0xa3, 0x56, 0x68, 0x72, 0x99, 0x8c, 0xc9,
	0xdd, 0xd0, 0xc8, 0xb3, 0xcf, 0xca, 0xa0, 0x7e, 0xed, 0xb8, 0x24, 0xd7,
	0xe5, 0x50, 0x94, 0x25, 0x20, 0xff, 0x12, 0x44, 0xf7, 0xf8, 0xea, 0xbc,
	0x3f, 0x26, 0x5d, 0x88, 0xce, 0x6a, 0x32, 0xe0, 0x0c, 0x23, 0x4e, 0x4d,
	0x8e, 0xbe, 0x72, 0x4f, 0x00, 0xd6, 0x23, 0x02, 0x78, 0xf2, 0xb7, 0x99,
	0x0a, 0xd8, 0xb1, 0x65, 0xa8, 0x3e, 0xd9, 0x22, 0x70, 0x99, 0xb7, 0xca,
	0xe0, 0x68, 0xd4, 0xe6, 0x52, 0xa6, 0xb1, 0xb2, 0x21, 0xef, 0x42, 0x63,
	0x52, 0xe9, 0x0d, 0xff, 0x56, 0x85, 0x33, 0x9c, 0x94, 0x92, 0xe5, 0xdf,
	0xcc, 0xb9, 0x23, 0xc7, 0xd5, 0x5a, 0xe2, 0x6a, 0xe7, 0xe1, 0x8a, 0x92,
	0x6c, 0xe4, 0x40, 0x37, 0xa3, 0xb5, 0x2f, 0xe9, 0x35, 0x02, 0x03, 0x01,
	0x00, 0x01, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
	0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0xa3, 0x03,
	0xc9, 0x46, 0xf6, 0xbd, 0x29, 0xd8, 0xd9, 0xb0, 0xbe, 0x18, 0xa9, 0x89,
	0xd2, 0x11, 0xe1, 0x23, 0x20, 0x18, 0xcd, 0xb7, 0x92, 0x89, 0xea, 0x92,
	0x19, 0x3a, 0x83, 0xd3, 0xc9, 0x21, 0xd9, 0x4a, 0x07, 0xaf, 0xb9, 0x7a,
	0xaa, 0xff, 0xa3, 0xc5, 0x81, 0x87, 0xf1, 0x21, 0x02, 0x95, 0x83, 0x27,
	0x59, 0x7e, 0xca, 0xc8, 0x7d, 0xce, 0x32, 0x00, 0x0b, 0x0d, 0x63, 0xf0,
	0x0f, 0x18, 0x2d, 0x83, 0xcc, 0x6b, 0x27, 0x1f, 0xa5, 0xd3, 0xd5, 0xb8,
	0x41, 0xf9, 0xcc, 0x68, 0xe8, 0x07, 0xd1, 0x05, 0xa2, 0xf3, 0x12, 0x57,
	0xf5, 0x52, 0xf0, 0x2b, 0x93, 0xe0, 0x96, 0x11, 0xb7, 0x3a, 0x36, 0x93,
	0xc5, 0xec, 0x47, 0xfe, 0x52, 0x65, 0xd4, 0x0b, 0x4f, 0xe7, 0x8e, 0xd9,
	0x72, 0x7f, 0xf5, 0x18, 0x8d, 0x83, 0x53, 0x07, 0xbe, 0xe8, 0x0c, 0xb0,
	0x9e, 0x38, 0xe5, 0x0d, 0xbd, 0x8f, 0x84, 0xe3, 0x51, 0x50, 0x0d, 0xe8,
	0x0c, 0x97, 0xf3, 0x73, 0x21, 0x3a, 0x7e, 0xdf, 0x17, 0x6f, 0x85, 0xab,
	0x33, 0xe2, 0x24, 0x48, 0x62, 0x19, 0x12, 0xeb, 0xc7, 0x87, 0xed, 0x00,
	0x92, 0xf6, 0x56, 0x49, 0x7b, 0x2d, 0xcf, 0x42, 0x2b, 0x1f, 0xb9, 0xc9,
	0x49, 0x2a, 0x64, 0x66, 0xfb, 0xb1, 0x97, 0xf1, 0xc2, 0x7f, 0xee, 0x14,
	0x79, 0xb5, 0x1f, 0xa1, 0x29, 0x55, 0xcb, 0xd9, 0x21, 0x6a, 0x5c, 0x2b,
	0xd6, 0x11, 0x3a, 0x02, 0x7a, 0x20, 0x9e, 0xc1, 0xca, 0xed, 0xc9, 0xc2,
	0x75, 0xc0, 0xca, 0x8f, 0x44, 0x0d, 0x32, 0x17, 0x9e, 0xf9, 0xc7, 0x9c,
	0xdf, 0x44, 0x02, 0xf2, 0x1a, 0xbb, 0xeb, 0x37, 0xd9, 0x4b, 0xab, 0xd7,
	0x0f, 0x79, 0xe2, 0xbc, 0xbc, 0x94, 0x8e, 0x97, 0x11, 0xe6, 0xed, 0xd0,
	0x62, 0x0a, 0x8f, 0xec, 0x38, 0x42, 0x44, 0x5c, 0x11, 0xc8, 0x21, 0x13,
	0x8d, 0x77
};

static unsigned char defpkey[] = {
	0x30, 0x82, 0x04, 0xbe, 0x02, 0x01, 0x00, 0x30, 0x0d, 0x06, 0x09, 0x2a,
	0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x82,
	0x04, 0xa8, 0x30, 0x82, 0x04, 0xa4, 0x02, 0x01, 0x00, 0x02, 0x82, 0x01,
	0x01, 0x00, 0xb9, 0xcd, 0x8b, 0x41, 0xdb, 0x05, 0xd5, 0x6c, 0x63, 0x6e,
	0xc7, 0x88, 0x8c, 0xb1, 0xa1, 0x50, 0x79, 0x27, 0xc3, 0xda, 0x97, 0xed,
	0xaf, 0xe8, 0x19, 0xf8, 0xab, 0x3f, 0x66, 0x84, 0x7f, 0x92, 0x9e, 0x47,
	0x8a, 0x8d, 0x6b, 0xa4, 0x7e, 0x54, 0x34, 0xe7, 0x1f, 0xe1, 0xf8, 0x15,
	0xcc, 0x4f, 0x61, 0xef, 0x0f, 0x0a, 0xea, 0x41, 0xe3, 0x9f, 0x8d, 0x55,
	0x85, 0xeb, 0x1f, 0xde, 0xde, 0x45, 0xc9, 0x12, 0xe1, 0x78, 0xa7, 0x6d,
	0xbb, 0x16, 0xfc, 0xd2, 0xe2, 0x53, 0xa7, 0xb2, 0x8f, 0xa6, 0xe0, 0x32,
	0xdc, 0xf0, 0x24, 0xe9, 0x82, 0xba, 0xdb, 0xce, 0xa4, 0x31, 0x77, 0x85,
	0x45, 0xea, 0x6a, 0x50, 0x87, 0x71, 0xff, 0xf4, 0x8c, 0x28, 0x18, 0x50,
	0x8d, 0x44, 0x96, 0x93, 0xb5, 0x72, 0xff, 0x47, 0x36, 0xa2, 0x60, 0x4d,
	0x2a, 0xff, 0xe1, 0xb0, 0x73, 0xa8, 0x83, 0xc4, 0xb5, 0x82, 0xd0, 0x3a,
	0x56, 0x05, 0x68, 0x57, 0xd4, 0x22, 0x12, 0x42, 0x7b, 0xab, 0x65, 0xf3,
	0x1b, 0xc4, 0xa3, 0x56, 0x68, 0x72, 0x99, 0x8c, 0xc9, 0xdd, 0xd0, 0xc8,
	0xb3, 0xcf, 0xca, 0xa0, 0x7e, 0xed, 0xb8, 0x24, 0xd7, 0xe5, 0x50, 0x94,
	0x25, 0x20, 0xff, 0x12, 0x44, 0xf7, 0xf8, 0xea, 0xbc, 0x3f, 0x26, 0x5d,
	0x88, 0xce, 0x6a, 0x32, 0xe0, 0x0c, 0x23, 0x4e, 0x4d, 0x8e, 0xbe, 0x72,
	0x4f, 0x00, 0xd6, 0x23, 0x02, 0x78, 0xf2, 0xb7, 0x99, 0x0a, 0xd8, 0xb1,
	0x65, 0xa8, 0x3e, 0xd9, 0x22, 0x70, 0x99, 0xb7, 0xca, 0xe0, 0x68, 0xd4,
	0xe6, 0x52, 0xa6, 0xb1, 0xb2, 0x21, 0xef, 0x42, 0x63, 0x52, 0xe9, 0x0d,
	0xff, 0x56, 0x85, 0x33, 0x9c, 0x94, 0x92, 0xe5, 0xdf, 0xcc, 0xb9, 0x23,
	0xc7, 0xd5, 0x5a, 0xe2, 0x6a, 0xe7, 0xe1, 0x8a, 0x92, 0x6c, 0xe4, 0x40,
	0x37, 0xa3, 0xb5, 0x2f, 0xe9, 0x35, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02,
	0x82, 0x01, 0x00, 0x3d, 0x7d, 0x79, 0xbd, 0xc4, 0xb7, 0x6c, 0x87, 0x5f,
	0x6c, 0xd5, 0x1a, 0x2a, 0xbd, 0xca, 0x8e, 0x30, 0x5a, 0x69, 0xa5, 0xd9,
	0x2b, 0xef, 0x50, 0x57, 0xf0, 0x04, 0xf4, 0x89, 0x80, 0x06, 0x1c, 0x46,
	0xc7, 0x94, 0x61, 0xf3, 0x22, 0xd0, 0x30, 0x1c, 0x05, 0xf5, 0x76, 0x3a,
	0x3d, 0x34, 0x8d, 0x7c, 0xf2, 0xb9, 0xbe, 0xba, 0x02, 0x2e, 0x4b, 0xaa,
	0x61, 0x88, 0x5d, 0x65, 0x01, 0xc3, 0xfb, 0xc2, 0x6c, 0xbe, 0x33, 0xc8,
	0xed, 0x36, 0xc2, 0x87, 0xcf, 0x5e, 0xd9, 0xa2, 0xa1, 0x24, 0x8b, 0x6c,
	0x9d, 0x48, 0xa3, 0x2d, 0x04, 0x1e, 0xa0, 0xaa, 0x87, 0xfb, 0xac, 0xe7,
	0x4a, 0x32, 0xf6, 0x3e, 0x53, 0x02, 0x50, 0x47, 0x0d, 0xee, 0xa8, 0x82,
	0x4b, 0x7e, 0x45, 0x19, 0xb4, 0x2e, 0x93, 0xad, 0x4a, 0x2b, 0x19, 0x97,
	0xd7, 0x52, 0x08, 0xb6, 0xef, 0x47, 0x87, 0x98, 0x7c, 0x2c, 0x39, 0x5a,
	0x1b, 0x03, 0x94, 0xd0, 0x2a, 0x94, 0x7a, 0x83, 0xfd, 0x07, 0x04, 0xf6,
	0x36, 0xf2, 0x62, 0x64, 0x98, 0xf3, 0x94, 0x9e, 0x40, 0x02, 0xed, 0x3a,
	0x58, 0xfc, 0x4e, 0x9f, 0x7d, 0xdc, 0xd5, 0x1f, 0x70, 0xd9, 0xe7, 0x2a,
	0x6e, 0x19, 0x47, 0xff, 0x87, 0x02, 0x3d, 0xfe, 0xdc, 0x6a, 0x8a, 0x09,
	0x83, 0x34, 0x68, 0x8c, 0xb7, 0x77, 0x71, 0x53, 0x09, 0xee, 0xdf, 0xb9,
	0x6a, 0x15, 0xdd, 0xf3, 0x8d, 0x9d, 0x43, 0x8a, 0xa9, 0x03, 0x1d, 0xf6,
	0x5e, 0x8e, 0x45, 0x0e, 0xd1, 0xf6, 0xdb, 0x9d, 0x30, 0x7a, 0x32, 0x7e,
	0xb8, 0xc7, 0x5a, 0xc3, 0x4f, 0x09, 0xce, 0x5a, 0x96, 0xba, 0x00, 0x4e,
	0x1d, 0xeb, 0x19, 0xe9, 0xca, 0xe6, 0xa6, 0x39, 0x58, 0x3f, 0x48, 0xca,
	0x18, 0x64, 0xb4, 0x0a, 0x05, 0xc0, 0x47, 0x32, 0xd5, 0x9d, 0x45, 0xa5,
	0x55, 0x43, 0xa0, 0x82, 0x3a, 0xf4, 0xe1, 0x02, 0x81, 0x81, 0x00, 0xf7,
	0xb6, 0xa8, 0x72, 0x71, 0xc0, 0x5d, 0x99, 0x1c, 0x0f, 0x4f, 0x1c, 0xc9,
	0x58, 0xf8, 0x13, 0x10, 0xfd, 0x36, 0x97, 0x78, 0xb9, 0xb5, 0xf2, 0x2b,
	0x1a, 0x6a, 0x6c, 0x67, 0xd1, 0x0e, 0xb7, 0x2e, 0xaa, 0x47, 0x5e, 0x16,
	0xdc, 0xc1, 0x7b, 0x40, 0x72, 0xb4, 0xa6, 0xa5, 0xca, 0xaa, 0xb4, 0x94,
	0xf9, 0xfc, 0x8b, 0xc6, 0xbf, 0xaf, 0x5f, 0xfa, 0x2e, 0x7f, 0x6d, 0xf3,
	0xfa, 0x98, 0x0a, 0xa2, 0x7e, 0x4a, 0x7b, 0x13, 0xe5, 0x57, 0x53, 0x11,
	0xdc, 0x7a, 0x6f, 0xc4, 0x68, 0x79, 0x44, 0x86, 0xc9, 0xc8, 0x36, 0x2e,
	0x77, 0x9b, 0xcc, 0xab, 0xa1, 0x34, 0xf7, 0x15, 0x8c, 0xed, 0x4b, 0xf0,
	0x3f, 0x9c, 0xaa, 0x75, 0xa8, 0x6b, 0x1d, 0x9e, 0xd5, 0x4d, 0x50, 0xf2,
	0x0d, 0x88, 0x5b, 0x5c, 0x9c, 0x4e, 0x3e, 0xea, 0x67, 0xf1, 0x57, 0x83,
	0x19, 0xd0, 0x16, 0x18, 0x09, 0x91, 0xed, 0x02, 0x81, 0x81, 0x00, 0xc0,
	0x04, 0xb3, 0xe4, 0x0d, 0xe9, 0x00, 0xfc, 0x52, 0x23, 0xfb, 0xb4, 0x24,
	0xa8, 0xac, 0x86, 0x3b, 0x00, 0x23, 0x2a, 0x81, 0xc1, 0xc5, 0xaf, 0x99,
	0xd9, 0xf9, 0x76, 0x21, 0xc1, 0xcd, 0x42, 0x0a, 0xbc, 0x72, 0x31, 0x99,
	0xd9, 0xb7, 0x66, 0xe5, 0x0d, 0xf3, 0x3d, 0x8a, 0x92, 0x69, 0xd2, 0xf1,
	0xd4, 0x81, 0x70, 0x8d, 0x30, 0x5b, 0xc1, 0xb8, 0x2d, 0x1b, 0xc3, 0x0b,
	0xdc, 0x16, 0x00, 0x34, 0xb5, 0xfa, 0x28, 0x9f, 0x5f, 0x5a, 0xe1, 0xf4,
	0xa5, 0x4c, 0x0f, 0x0c, 0x44, 0xc1, 0xb4, 0x48, 0x53, 0x8f, 0x8c, 0x69,
	0xfc, 0xb7, 0x5d, 0xc2, 0x63, 0x40, 0x72, 0x6e, 0xaa, 0x1c, 0x76, 0xcb,
	0x50, 0x5f, 0x28, 0x20, 0x27, 0x2d, 0xe9, 0x11, 0x24, 0x6d, 0xf9, 0xc8,
	0xcb, 0xd4, 0xbf, 0x29, 0xd1, 0x84, 0x26, 0xb7, 0x2d, 0x03, 0xca, 0x6f,
	0x08, 0x1b, 0x2a, 0x59, 0xb9, 0x6b, 0x69, 0x02, 0x81, 0x81, 0x00, 0xdb,
	0xc0, 0x3b, 0xe1, 0x03, 0x0e, 0x8f, 0x3a, 0x13, 0xcd, 0x74, 0xf6, 0x69,
	0x9c, 0xb6, 0xbe, 0x5b, 0x54, 0xf6, 0xc8, 0x1b, 0x3e, 0x4c, 0xad, 0xa0,
	0x15, 0x58, 0x12, 0x01, 0x6b, 0x51, 0xad, 0xaa, 0x05, 0x3f, 0x38, 0xb5,
	0xf9, 0x72, 0xdf, 0x3f, 0x46, 0x43, 0x02, 0x8f, 0x93, 0xa2, 0x2a, 0x7e,
	0xfa, 0xe2, 0x52, 0xb1, 0xa7, 0x3d, 0x12, 0x5c, 0xe4, 0x41, 0x01, 0xae,
	0xee, 0x41, 0x51, 0x35, 0xe3, 0xe9, 0x79, 0x14, 0x26, 0x61, 0x78, 0x7d,
	0x20, 0xe5, 0xb1, 0x1f, 0x41, 0x3b, 0x4b, 0x52, 0x73, 0xd1, 0x5c, 0x57,
	0x8e, 0x9e, 0x35, 0x4c, 0x90, 0xbf, 0x66, 0xee, 0xc4, 0x4d, 0x83, 0x0f,
	0x87, 0xaf, 0x18, 0xfd, 0x35, 0xc2, 0x88, 0xcd, 0x6e, 0x16, 0x9f, 0xd7,
	0x8b, 0xaf, 0xca, 0xb4, 0xf6, 0xa8, 0xe7, 0x61, 0x95, 0x82, 0x61, 0x8b,
	0x19, 0x2c, 0x23, 0x5d, 0x81, 0xa7, 0xe5, 0x02, 0x81, 0x81, 0x00, 0xa5,
	0x5e, 0xe7, 0x76, 0xcf, 0x2c, 0x0e, 0xb7, 0x40, 0x1e, 0xd1, 0x55, 0xf2,
	0x04, 0x7a, 0xa1, 0x18, 0x0b, 0x24, 0x3e, 0x25, 0x86, 0x36, 0xb4, 0xe7,
	0x31, 0xbd, 0x10, 0xaa, 0x3d, 0xf8, 0x82, 0x9a, 0x30, 0x75, 0xc0, 0x96,
	0x19, 0xd5, 0x43, 0xfe, 0x9d, 0x27, 0x11, 0xa7, 0x58, 0x35, 0x5f, 0x80,
	0xfa, 0x5d, 0x88, 0x0b, 0x70, 0xcc, 0x75, 0x68, 0x23, 0x21, 0x37, 0xb7,
	0xed, 0x11, 0xdd, 0x97, 0x87, 0xea, 0x2c, 0x36, 0x25, 0xaf, 0x65, 0xce,
	0x86, 0xd5, 0x9c, 0x58, 0x1a, 0x97, 0x20, 0xa6, 0xf1, 0xa0, 0x3d, 0x73,
	0xc2, 0x3c, 0x41, 0xb9, 0x7a, 0x35, 0x34, 0xd8, 0x6e, 0x15, 0x19, 0x92,
	0x3c, 0x63, 0xd7, 0x70, 0xe9, 0x8d, 0x36, 0x9e, 0x96, 0xdb, 0x48, 0x36,
	0x50, 0xa5, 0xa7, 0x05, 0x62, 0xf6, 0x07, 0x49, 0xd6, 0xb0, 0xdf, 0x15,
	0x04, 0xcc, 0x30, 0x8b, 0x19, 0x8f, 0x91, 0x02, 0x81, 0x80, 0x5a, 0x4c,
	0x01, 0x97, 0x62, 0xea, 0x59, 0x18, 0xd2, 0x62, 0xc6, 0xef, 0x38, 0xc6,
	0x8c, 0x6d, 0x1c, 0x92, 0xb0, 0x57, 0x1f, 0x5d, 0xf5, 0xa7, 0x31, 0xe0,
	0x00, 0x4a, 0x62, 0x83, 0x66, 0x81, 0xd0, 0x56, 0x85, 0xc9, 0x4e, 0x94,
	0x5c, 0xae, 0x55, 0xeb, 0x06, 0xd0, 0x43, 0x1a, 0x9c, 0x0e, 0xff, 0xbd,
	0x1f, 0x68, 0x19, 0x49, 0x8d, 0xfc, 0xb5, 0x42, 0x05, 0xcc, 0x36, 0xdc,
	0x4d, 0x63, 0x1e, 0x25, 0xfe, 0x69, 0x4f, 0xff, 0x11, 0xe0, 0xaf, 0x00,
	0xac, 0x44, 0x31, 0x5e, 0xa1, 0xd1, 0x40, 0x9d, 0x03, 0xe3, 0x75, 0x2a,
	0x9a, 0x75, 0x57, 0x87, 0x9d, 0x65, 0x17, 0xe5, 0xf9, 0x35, 0x3c, 0x9d,
	0xe1, 0xa4, 0xad, 0x78, 0x38, 0x16, 0x71, 0xb3, 0xee, 0x9b, 0x42, 0xa3,
	0xe8, 0x84, 0xd5, 0xce, 0x93, 0xc0, 0x6f, 0xc3, 0xe4, 0xe4, 0x3c, 0xec,
	0x0a, 0x25, 0x1f, 0x61, 0xdf, 0xac
};