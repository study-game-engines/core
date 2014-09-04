//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"
#include <EGL/egl.h>

std::string              g_sExtensions     = "";
thread_local coreContext g_CoreContext; // = 0;


// ****************************************************************  
/* init OpenGL ES */
void __coreInitOpenGLES()
{
    // reset context structure
    std::memset(&g_CoreContext, 0, sizeof(coreContext));

    // get full extension string
    if(g_sExtensions.empty()) g_sExtensions = r_cast<const char*>(glGetString(GL_EXTENSIONS));

    // get OpenGL ES version
    g_CoreContext.__fVersion = coreData::StrVersion(r_cast<const char*>(glGetString(GL_VERSION)));
    g_CoreContext.__bES3     = (g_CoreContext.__fVersion >= 3.0f);
    const bool&     bES3     =  g_CoreContext.__bES3;

    // implement GL_EXT_discard_framebuffer
    if(__CORE_GLES_CHECK(GL_EXT_discard_framebuffer, false))
    {
        __CORE_GLES_FUNC_FETCH(glDiscardFramebuffer, EXT, false)
    }
    else if(__CORE_GLES_CHECK(GL_EXT_discard_framebuffer, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glInvalidateFramebuffer, , bES3)
    }

    // implement GL_EXT_texture_storage
    if(__CORE_GLES_CHECK(GL_EXT_texture_storage, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glTexStorage2D, EXT, bES3)
    }

    // implement GL_OES_mapbuffer and GL_EXT_map_buffer_range
    if(__CORE_GLES_CHECK(GL_OES_mapbuffer, bES3))
    {
        if(__CORE_GLES_CHECK(GL_EXT_map_buffer_range, bES3))
        {
            __CORE_GLES_FUNC_FETCH(glMapBufferRange, EXT, bES3)
            __CORE_GLES_FUNC_FETCH(glUnmapBuffer,    OES, bES3)
        }
    }

    // implement GL_EXT_instanced_arrays
    if(__CORE_GLES_CHECK(GL_EXT_instanced_arrays, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glDrawArraysInstanced,   EXT, bES3)
        __CORE_GLES_FUNC_FETCH(glDrawElementsInstanced, EXT, bES3)
        __CORE_GLES_FUNC_FETCH(glVertexAttribDivisor,   EXT, bES3)
    }

    // implement GL_EXT_texture_filter_anisotropic
    __CORE_GLES_CHECK(GL_EXT_texture_filter_anisotropic, false);

    // implement GL_NV_pixel_buffer_object
    __CORE_GLES_CHECK(GL_NV_pixel_buffer_object, bES3);

    // implement GL_NV_framebuffer_blit
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_blit, bES3))
    {
        __CORE_GLES_VAR_SET(GL_READ_FRAMEBUFFER, 0x8CA8)
        __CORE_GLES_VAR_SET(GL_DRAW_FRAMEBUFFER, 0x8CA9)

        __CORE_GLES_FUNC_FETCH(glBlitFramebuffer, NV, bES3)
    }
    else
    {
        __CORE_GLES_VAR_SET(GL_READ_FRAMEBUFFER, GL_FRAMEBUFFER)
        __CORE_GLES_VAR_SET(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER)
    }

    // implement GL_NV_framebuffer_multisample
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_multisample, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glRenderbufferStorageMultisample, NV, bES3)
    }

    // implement GL_OES_vertex_array_object
    if(__CORE_GLES_CHECK(GL_OES_vertex_array_object, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glBindVertexArray,    OES, bES3)
        __CORE_GLES_FUNC_FETCH(glDeleteVertexArrays, OES, bES3)
        __CORE_GLES_FUNC_FETCH(glGenVertexArrays,    OES, bES3)
    }

    // implement GL_OES_depth_texture
    __CORE_GLES_CHECK(GL_OES_depth_texture, false);

    // implement GL_OES_texture_stencil8
    __CORE_GLES_CHECK(GL_OES_texture_stencil8, false);

    // implement GL_OES_rgb8_rgba8
    if(__CORE_GLES_CHECK(GL_OES_rgb8_rgba8, false))
    {
        __CORE_GLES_VAR_SET(GL_RGB8,  0x8051)
        __CORE_GLES_VAR_SET(GL_RGBA8, 0x8058)
    }
    else
    {
        __CORE_GLES_VAR_SET(GL_RGB8,  GL_RGB565)
        __CORE_GLES_VAR_SET(GL_RGBA8, GL_RGBA4)
    }
}