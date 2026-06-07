#ifndef __JAVA_JAR_INFO_H__
#define __JAVA_JAR_INFO_H__

int javaFeatureVersionFromClassMajor(int major);
int getRequiredJavaVersionFromJar(const char* jarPath);

#endif
