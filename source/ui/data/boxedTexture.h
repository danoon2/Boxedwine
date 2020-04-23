#ifndef __BOXED_TEXTURE_H__
#define __BOXED_TEXTURE_H__

class BoxedTexture {
public:
	BoxedTexture(std::function<void* ()> loadTexture);
	~BoxedTexture();

	static void resetAll();

	void* getTexture();	
private:
	void unload();

	void* texture;
	std::function<void* ()> loadTexture;
};

#endif