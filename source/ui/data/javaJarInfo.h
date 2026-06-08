#ifndef __JAVA_JAR_INFO_H__
#define __JAVA_JAR_INFO_H__

int javaFeatureVersionFromClassMajor(int major);
bool isJavaJarLaunchable(const char* jarPath);
int getRequiredJavaVersionFromJar(const char* jarPath);

#endif
