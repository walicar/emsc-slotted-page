#include <gtest/gtest.h>
#include "slotted_page.h"

DAS *marshal_text(std::string text);
std::string unmarshal_text(DAS &block);

namespace
{
    TEST(test_example, basic_check)
	{
		int value = 3 + 1;
		int expected = 4;
		ASSERT_EQ(value, expected);
	}

	TEST(slotted_page, slotted_page_basics) {
		SlottedPage *page = new SlottedPage();
		ASSERT_EQ(page->size(), 0);
		DAS *key = marshal_text("miles");
		DAS *value = marshal_text("davis");
		page->put(key, value);
		ASSERT_EQ(page->size(), 1);
		DAS *get = page->get(key);
		std::string get_value = unmarshal_text(*get);
		ASSERT_EQ("davis", get_value);
		delete key;
		delete value;
		delete page;
	}

	TEST(slotted_page, multiple_keys) {
		SlottedPage *page = new SlottedPage();
		ASSERT_EQ(page->size(), 0);
		DAS *a_key = marshal_text("miles");
		DAS *a_val = marshal_text("davis");
		page->put(a_key, a_val);
		ASSERT_EQ(page->size(), 1);
		DAS *a_get = page->get(a_key);
		std::string a_get_val = unmarshal_text(*a_get);
		ASSERT_EQ("davis", a_get_val);
		DAS *b_key = marshal_text("john");
		DAS *b_val = marshal_text("coltrane");
		page->put(b_key, b_val);
		ASSERT_EQ(page->size(), 2);
		DAS *b_get = page->get(b_key);
		std::string b_get_val = unmarshal_text(*b_get);
		ASSERT_EQ("coltrane", b_get_val);

		delete a_key;
		delete a_val;
		delete a_get;
		delete b_key;
		delete b_val;
		delete b_get;
		delete page;
	}
}

DAS *marshal_text(std::string text)
{
    uint size = text.length();
    uint offset = 0;

    char *bytes = new char[BLOCK_SZ];
    *(u_int16_t *)(bytes + offset) = size;
    offset += sizeof(u_int16_t);
    memcpy(bytes + offset, text.c_str(), size);
    offset += size;

    char *right_size_bytes = new char[offset];
    memcpy(right_size_bytes, bytes, offset);
    delete[] bytes;
    return new DAS(right_size_bytes, offset);
}

std::string unmarshal_text(DAS &data)
{
    char *bytes = (char *)data.get_data();
    u_int16_t size;
    memcpy(&size, bytes, sizeof(u_int16_t));
    std::string text(bytes + sizeof(u_int16_t), size);
    return text;
}