#ifndef __ARMV8BTOPS_SSE_CONVERT_H__
#define __ARMV8BTOPS_SSE_CONVERT_H__

void opCvtss2siR32Xmm(Armv8btAsm* data);
void opCvtss2siR32E32(Armv8btAsm* data);
void opCvttps2piMmxXmm(Armv8btAsm* data);
void opCvttps2piMmxE64(Armv8btAsm* data);
void opCvttss2siR32Xmm(Armv8btAsm* data);
void opCvttss2siR32E32(Armv8btAsm* data);
void opCvtps2piMmxXmm(Armv8btAsm* data);
void opCvtps2piMmxE64(Armv8btAsm* data);
void opCvtsi2ssXmmR32(Armv8btAsm* data);
void opCvtsi2ssXmmE32(Armv8btAsm* data);
void opCvtpd2piMmxXmm(Armv8btAsm* data);
void opCvtpd2piMmxE128(Armv8btAsm* data);
void opCvtpi2pdXmmMmx(Armv8btAsm* data);
void opCvtpi2pdXmmE64(Armv8btAsm* data);
void opCvtsd2siR32Xmm(Armv8btAsm* data);
void opCvtsd2siR32E64(Armv8btAsm* data);
void opCvtsi2sdXmmR32(Armv8btAsm* data);
void opCvtsi2sdXmmE32(Armv8btAsm* data);
void opCvttpd2piMmxXmm(Armv8btAsm* data);
void opCvttpd2piMmE128(Armv8btAsm* data);
void opCvttsd2siR32Xmm(Armv8btAsm* data);
void opCvttsd2siR32E64(Armv8btAsm* data);
void opCvtpi2psXmmMmx(Armv8btAsm* data);
void opCvtpi2psXmmE64(Armv8btAsm* data);
void opCvtpd2psXmmXmm(Armv8btAsm* data);
void opCvtpd2psXmmE128(Armv8btAsm* data);
void opCvtps2pdXmmXmm(Armv8btAsm* data);
void opCvtps2pdXmmE64(Armv8btAsm* data);
void opCvtsd2ssXmmXmm(Armv8btAsm* data);
void opCvtsd2ssXmmE64(Armv8btAsm* data);
void opCvtdq2pdXmmXmm(Armv8btAsm* data);
void opCvtdq2pdXmmE128(Armv8btAsm* data);
void opCvtdq2psXmmXmm(Armv8btAsm* data);
void opCvtdq2psXmmE128(Armv8btAsm* data);
void opCvtpd2dqXmmXmm(Armv8btAsm* data);
void opCvtpd2dqXmmE128(Armv8btAsm* data);
void opCvtps2dqXmmXmm(Armv8btAsm* data);
void opCvtps2dqXmmE128(Armv8btAsm* data);
void opCvtss2sdXmmXmm(Armv8btAsm* data);
void opCvtss2sdXmmE32(Armv8btAsm* data);
void opCvttpd2dqXmmXmm(Armv8btAsm* data);
void opCvttpd2dqXmmE128(Armv8btAsm* data);
void opCvttps2dqXmmXmm(Armv8btAsm* data);
void opCvttps2dqXmmE128(Armv8btAsm* data);
#endif