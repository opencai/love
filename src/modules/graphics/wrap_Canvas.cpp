/**
 * Copyright (c) 2006-2020 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "wrap_Canvas.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{

Canvas *luax_checkcanvas(lua_State *L, int idx)
{
	return luax_checktype<Canvas>(L, idx);
}

int w_Canvas_renderTo(lua_State *L)
{
	Graphics::RenderTarget rt(luax_checkcanvas(L, 1));

	int startidx = 2;

	if (rt.texture->getTextureType() != TEXTURE_2D)
	{
		rt.slice = (int) luaL_checkinteger(L, 2) - 1;
		startidx++;
	}

	luaL_checktype(L, startidx, LUA_TFUNCTION);

	auto graphics = Module::getInstance<Graphics>(Module::M_GRAPHICS);

	if (graphics)
	{
		// Save the current render targets so we can restore them when we're done.
		Graphics::RenderTargets oldtargets = graphics->getCanvas();

		for (auto c : oldtargets.colors)
			c.texture->retain();

		if (oldtargets.depthStencil.texture != nullptr)
			oldtargets.depthStencil.texture->retain();

		luax_catchexcept(L, [&](){ graphics->setCanvas(rt, false); });

		lua_settop(L, 2); // make sure the function is on top of the stack
		int status = lua_pcall(L, 0, 0, 0);

		graphics->setCanvas(oldtargets);

		for (auto c : oldtargets.colors)
			c.texture->release();

		if (oldtargets.depthStencil.texture != nullptr)
			oldtargets.depthStencil.texture->release();

		if (status != 0)
			return lua_error(L);
	}
	
	return 0;
}

int w_Canvas_newImageData(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	love::image::Image *image = luax_getmodule<love::image::Image>(L, love::image::Image::type);

	int slice = 0;
	int mipmap = 0;
	Rect rect = {0, 0, canvas->getPixelWidth(), canvas->getPixelHeight()};

	if (canvas->getTextureType() != TEXTURE_2D)
		slice = (int) luaL_checkinteger(L, 2) - 1;

	mipmap = (int) luaL_optinteger(L, 3, 1) - 1;

	if (!lua_isnoneornil(L, 4))
	{
		rect.x = (int) luaL_checkinteger(L, 4);
		rect.y = (int) luaL_checkinteger(L, 5);
		rect.w = (int) luaL_checkinteger(L, 6);
		rect.h = (int) luaL_checkinteger(L, 7);
	}

	love::image::ImageData *img = nullptr;
	luax_catchexcept(L, [&](){ img = canvas->newImageData(image, slice, mipmap, rect); });

	luax_pushtype(L, img);
	img->release();
	return 1;
}

int w_Canvas_getMipmapMode(lua_State *L)
{
	Canvas *c = luax_checkcanvas(L, 1);
	const char *str;
	if (!Texture::getConstant(c->getMipmapsMode(), str))
		return luax_enumerror(L, "mipmap mode", Texture::getConstants(Texture::MIPMAPS_MAX_ENUM), str);

	lua_pushstring(L, str);
	return 1;
}

static const luaL_Reg w_Canvas_functions[] =
{
	{ "renderTo", w_Canvas_renderTo },
	{ "newImageData", w_Canvas_newImageData },
	{ "getMipmapMode", w_Canvas_getMipmapMode },
	{ 0, 0 }
};

extern "C" int luaopen_canvas(lua_State *L)
{
	return luax_register_type(L, &Canvas::type, w_Texture_functions, w_Canvas_functions, nullptr);
}

} // graphics
} // love
