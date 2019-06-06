#include "DetourDebugInterface.h"
#include "GL/glew.h"
#include "DebugDraw.h" 
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleProgram.h"

class GLCheckerTexture
{
	unsigned int m_texId;
public:
	GLCheckerTexture() : m_texId(0)
	{
	}

	~GLCheckerTexture()
	{
		if (m_texId != 0)
			glDeleteTextures(1, &m_texId);
	}
	void bind()
	{
		if (m_texId == 0)
		{
			// Create checker pattern.
			const unsigned int col0 = duRGBA(215, 215, 215, 255);
			const unsigned int col1 = duRGBA(255, 255, 255, 255);
			static const int TSIZE = 64;
			unsigned int data[TSIZE*TSIZE];

			glGenTextures(1, &m_texId);
			glBindTexture(GL_TEXTURE_2D, m_texId);

			int level = 0;
			int size = TSIZE;
			while (size > 0)
			{
				for (int y = 0; y < size; ++y)
					for (int x = 0; x < size; ++x)
						data[x + y * size] = (x == 0 || y == 0) ? col0 : col1;
				glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				size /= 2;
				level++;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_texId);
		}
	}
};
GLCheckerTexture g_tex;

enum SamplePolyAreas
{
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP,
};

DetourDebugInterface::DetourDebugInterface()
{
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	shader = App->program->GetProgram("DebugNav");
}

DetourDebugInterface::~DetourDebugInterface()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	vertices.clear();
	App->resManager->DeleteProgram("DebugNav");
}

unsigned int DetourDebugInterface::areaToCol(unsigned int area)
{
	switch (area)
	{
		// Ground (0) : light blue
	case SAMPLE_POLYAREA_GROUND: return duRGBA(0, 192, 255, 255);
		// Water : blue
	case SAMPLE_POLYAREA_WATER: return duRGBA(0, 0, 255, 255);
		// Road : brown
	case SAMPLE_POLYAREA_ROAD: return duRGBA(50, 20, 12, 255);
		// Door : cyan
	case SAMPLE_POLYAREA_DOOR: return duRGBA(0, 255, 255, 255);
		// Grass : green
	case SAMPLE_POLYAREA_GRASS: return duRGBA(0, 255, 0, 255);
		// Jump : yellow
	case SAMPLE_POLYAREA_JUMP: return duRGBA(255, 255, 0, 255);
		// Unexpected : red
	default: return duRGBA(255, 0, 0, 255);
	}
}

void DetourDebugInterface::depthMask(bool state)
{
	depth = state;
}

void DetourDebugInterface::texture(bool state)
{
	if (state)
	{
		glEnable(GL_TEXTURE_2D);
		g_tex.bind();
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}

void DetourDebugInterface::begin(duDebugDrawPrimitives prim, float size)
{
	currentShape = new Shape;
	switch (prim)
	{
	case DU_DRAW_POINTS:
		currentShape->drawMode = GL_POINTS;
		break;
	case DU_DRAW_LINES:
		currentShape->drawMode = GL_LINES;
		break;
	case DU_DRAW_TRIS:
		currentShape->drawMode = GL_TRIANGLES;
		break;
	case DU_DRAW_QUADS:
		currentShape->drawMode = GL_QUADS;
		break;
	};
	currentShape->start = vertices.size();
	currentShape->size = size;
}

math::float4 DetourDebugInterface::getColor(unsigned col)
{
	unsigned r = col & 0xff;
	unsigned g = (col >> 8) & 0xff;
	unsigned b = (col >> 16) & 0xff;
	unsigned a = (col >> 24) & 0xff;
	float f = 1.0f / 255.0f;
	return math::float4(r, g, b, a) * f;
}

void DetourDebugInterface::vertex(const float* pos, unsigned int color)
{
	v3_c4 vertex = { math::float3(pos), getColor(color) };
	vertices.push_back(vertex);
}

void DetourDebugInterface::vertex(const float x, const float y, const float z, unsigned int color)
{
	v3_c4 vertex = { math::float3(x,y,z), getColor(color) };
	vertices.push_back(vertex);
}

void DetourDebugInterface::vertex(const float* pos, unsigned int color, const float* uv) //TODO: Update to GL3.3
{
	glColor4ubv((GLubyte*)&color);
	glTexCoord2fv(uv);
	glVertex3fv(pos);
}

void DetourDebugInterface::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
	glColor4ubv((GLubyte*)&color);
	glTexCoord2f(u, v);
	glVertex3f(x, y, z);
	//TODO: Update to GL3.3
}

void DetourDebugInterface::debugDrawNavMesh()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(shader->id[0]);
	math::float4x4 model = math::float4x4::identity;
	glUniformMatrix4fv(glGetUniformLocation(shader->id[0], "model"), 1, GL_TRUE, &model[0][0]);

	glBindVertexArray(VAO);

	for (const auto& shape : shapes)
	{
		if (shape->drawMode == GL_LINES) {
			glLineWidth(shape->size);
		}

		glDrawArrays(shape->drawMode, shape->start, shape->end - shape->start);
	}

	glLineWidth(1.0f);
	glBindVertexArray(0);
	glUseProgram(0);
	GLenum err;
	/*while ((err = glGetError()) != GL_NO_ERROR)
	{
		LOG("GLEW Error: %s",glewGetErrorString(err));
	}*/
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void DetourDebugInterface::end()
{
	if (vertices.size() == 0) return;
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v3_c4)* vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,  // attribute
		3,                  // number of elements per vertex, here (x,y,z)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		sizeof(v3_c4),                  // no extra data between each position
		(void*)0                  // offset of first element
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,  // attribute
		4,                  // number of elements per vertex, here (x,y,z)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		sizeof(v3_c4),                  // no extra data between each position
		(void*)sizeof(math::float3)     // offset of first element
	);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	currentShape->end = vertices.size();
	shapes.push_back(currentShape);
	currentShape = nullptr;
}