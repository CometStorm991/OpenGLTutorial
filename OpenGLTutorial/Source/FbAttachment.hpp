#pragma once

#include <cstdint>

#include <GL/glew.h>

class FbAttachment
{
public:
	FbAttachment(GLenum attachmentPt, GLenum targetType, uint32_t attachmentId);

	GLenum getAttachmentPt();
	GLenum getTargetType();
	uint32_t getAttachmentId();
private:
	GLenum attachmentPt;
	GLenum targetType;
	uint32_t attachmentId;
};