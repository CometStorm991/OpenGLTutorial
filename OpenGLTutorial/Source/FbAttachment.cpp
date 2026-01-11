#include "FbAttachment.hpp"

FbAttachment::FbAttachment(GLenum attachmentPt, GLenum targetType, uint32_t attachmentId)
	: attachmentPt(attachmentPt), targetType(targetType), attachmentId(attachmentId)
{

}

GLenum FbAttachment::getAttachmentPt()
{
	return attachmentPt;
}

GLenum FbAttachment::getTargetType()
{
	return targetType;
}

GLenum FbAttachment::getAttachmentId()
{
	return attachmentId;
}