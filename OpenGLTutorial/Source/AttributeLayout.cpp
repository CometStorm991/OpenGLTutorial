#include "AttributeLayout.hpp"

AttributeLayout::AttributeLayout(uint32_t count, GLenum type)
	: count(count), type(type)
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