#include "boxedwine.h"
#include "../boxedwineui.h"

void UnloadTexture(void* texture);

static std::set<BoxedTexture*> textures;

void BoxedTexture::resetAll() {
	for (auto& t : textures) {
		t->unload();
	}
}

BoxedTexture::BoxedTexture(std::function<void* ()> loadTexture) : texture(nullptr), loadTexture(loadTexture) {
	textures.insert(this);
}

BoxedTexture::~BoxedTexture() {
	unload();
	textures.erase(this);
}

void* BoxedTexture::getTexture() {
	if (!texture) {
		texture = this->loadTexture();
	}
	return texture;
}

void BoxedTexture::unload() {
	if (texture) {
		UnloadTexture(texture);
		texture = nullptr;
	}
}