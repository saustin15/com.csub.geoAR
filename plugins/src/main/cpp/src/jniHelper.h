/*
 * jniHelper.h
 *
 *  Created on: Aug 22, 2011
 *      Author: wolfgang
 */

#ifndef JNIHELPER_H
#define JNIHELPER_H

#include <jni.h>
#include <string>

#define JNI_REFS_DEFAULT 64

/**
 * Helper class to manage the attaching and detaching to a java virtual machine environment automatically.
 *
 * Just create the JavaVMResource object on the stack and it will take care of detaching when the function returns.
 */
class JavaVMResource {
public:
	/**
	 * Attaches a java virtual machine environment.
	 */
	JavaVMResource();

	/**
	 * Detaches the current thread from the java virtual machine.
	 */
	~JavaVMResource();

	JNIEnv* env;
	static JavaVM*  JAVA_VM;

private:
	bool isAttached;
};

class JavaStringResource {
public:
	/**
	 * Load the java string
	 */
	JavaStringResource(JNIEnv* env, const jstring string);

	/**
	 * Makes sure the java string is released
	 */
	~JavaStringResource();

	std::string str;

private:
	JNIEnv* env;
	jstring javaString;
	const char *javaStrPtr;
};

#endif
