#ifndef GKIT_UTILS_H
#define GKIT_UTILS_H

#include <GL/GLBuffer.h>
#include <GL/GLVertexArray.h>

struct GLVertexBufferSet
{
	gk::GLVertexArray* vao;

	gk::GLBuffer* positions;
	gk::GLBuffer* colors;
	gk::GLBuffer* indices;

	unsigned int indexcount;

	GLVertexBufferSet()
		:vao(nullptr),
		positions(nullptr),
		colors(nullptr),
		indices(nullptr),
		indexcount(0)
	{
	}
	~GLVertexBufferSet()
	{
		if (vao)
		{
			vao->release();
			delete vao;
			vao = nullptr;
		}

		if (positions)
		{
			positions->release();
			delete positions;
			positions = nullptr;
		}
		if (colors)
		{
			colors->release();
			delete colors;
			colors = nullptr;
		}
		if (indices)
		{
			indices->release();
			delete indices;
			indices = nullptr;
		}

		indexcount = 0;
	}
};

//! fonction utilitaire :  creation d'un vertex array non managé.
//! \ingroup OpenGL.
inline
gk::GLVertexArray *createGLUnmanagedVertexArray()
{
	gk::GLVertexArray* glvao = new gk::GLVertexArray();

	assert(glvao->name == 0 && "vertex array creation error");

	glGenVertexArrays(1, &glvao->name);
	if (glvao->name == 0)
		return glvao;

	glBindVertexArray(glvao->name);

	return glvao;
}
//! fonction utilitaire : creation simplifiee d'un buffer initialise. utilise glBufferData pour construire le buffer.
//! \ingroup OpenGL.
template < typename T >
gk::GLBuffer *createGLUnmanagedBuffer(const GLenum target, const std::vector<T>& data,
	const GLenum usage = GL_STATIC_DRAW)
{
	const unsigned int _length = data.size() * sizeof(T);

	gk::GLBuffer* glbuffer = new gk::GLBuffer();

	assert(glbuffer->name == 0 && "create buffer error");
	glGenBuffers(1, &glbuffer->name);
	if (glbuffer->name == 0)
		return glbuffer;

	glbuffer->length = _length;
	glbuffer->storage = 0;
	glbuffer->map = NULL;

	glBindBuffer(target, glbuffer->name);

	//~ if(GLEW_ARB_debug_output)
	//~ glObjectLabel(GL_BUFFER, name, -1, "'buffer gKit'");    // ok

	glBufferData(target, glbuffer->length, &data.front(), usage);

	return glbuffer;
}

#endif