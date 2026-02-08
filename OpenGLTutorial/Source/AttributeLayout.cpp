#include "AttributeLayout.hpp"

AttributeLayout::AttributeLayout(uint32_t count, GLenum type) : AttributeLayout(count, type, 0)
{

}

AttributeLayout::AttributeLayout(uint32_t count, GLenum type, uint32_t layoutIndex)
	: count(count), type(type), layoutIndex(layoutIndex)
{

}

uint32_t AttributeLayout::getCount()
{
	return count;
}

GLenum AttributeLayout::getType()
{
	return type;
}