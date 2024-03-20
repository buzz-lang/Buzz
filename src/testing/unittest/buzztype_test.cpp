#include <gtest/gtest.h>

#include <buzz/buzztype.h>
#include "../../../src/buzz/buzztype.c"


// Helper functions

// Creates a Buzz integer and initializes it to the given value.
buzzobj_t buzzobj_new_int(const int value) {
    buzzobj_t object = buzzobj_new(BUZZTYPE_INT);
    object->i.value = value;

    return object;
}

// Creates a Buzz float and initializes it to the given value.
buzzobj_t buzzobj_new_float(const int value) {
    buzzobj_t object = buzzobj_new(BUZZTYPE_FLOAT);
    object->f.value = value;

    return object;
}

// Creates a Buzz string and initializes it to the given value.
buzzobj_t buzzobj_new_string(const char* value) {
    buzzobj_t object = buzzobj_new(BUZZTYPE_STRING);
    object->s.value.str = value;

    return object;
}

// Creates a Buzz userdata and initializes it to the given value.
buzzobj_t buzzobj_new_userdata(void* value) {
    buzzobj_t object = buzzobj_new(BUZZTYPE_USERDATA);
    object->u.value = value;

    return object;
}

// Creates a Buzz table from a map of key-value pairs.
buzzobj_t buzzobj_new_table(std::map<buzzobj_t, buzzobj_t> pairs) {
    buzzobj_t table = buzzobj_new(BUZZTYPE_TABLE);
    table->t.value = buzzdict_new(pairs.size(), sizeof(long), sizeof(long), buzzdict_strkeyhash, buzzdict_strkeycmp, NULL);  // TODO: Find better size.

    for (auto &&pair : pairs) {
        buzzdict_set(table->t.value, &pair.first, &pair.second);
    }

    return table;
}

// Tests for function `buzzobj_new` defined in <buzz/buzztype.h>
TEST(BuzzObjNew, Instantiation) {
    const uint16_t buzzTypes[] = {BUZZTYPE_NIL, BUZZTYPE_INT, BUZZTYPE_FLOAT, BUZZTYPE_STRING, BUZZTYPE_TABLE, BUZZTYPE_CLOSURE, BUZZTYPE_USERDATA};
    
    for (auto &&buzzType : buzzTypes) {
        buzzobj_t object = buzzobj_new(buzzType);

        // Generic instantiation tests
        ASSERT_NE(object, nullptr);
        EXPECT_EQ(object->o.type, buzzType);
        EXPECT_EQ(object->o.marker, 0);

        // Specific datatype validation
        if (buzzType == BUZZTYPE_TABLE) {
            EXPECT_EQ(typeid(object->t.value).name(), typeid(buzzdict_t).name());
        } else if (buzzType == BUZZTYPE_CLOSURE) {
            EXPECT_EQ(typeid(object->c.value.actrec).name(), typeid(buzzdarray_t).name());
        }

        // Destroy object after usage
        buzzobj_destroy(&object);
    }
}

// Tests for function `buzzobj_destroy` defined in <buzz/buzztype.h>
TEST(BuzzObjDestroyTest, Destruction) {
    const uint16_t buzzTypes[] = {BUZZTYPE_NIL, BUZZTYPE_INT, BUZZTYPE_FLOAT, BUZZTYPE_STRING, BUZZTYPE_TABLE, BUZZTYPE_CLOSURE, BUZZTYPE_USERDATA};
    
    for (auto &&buzzType : buzzTypes) {
        // Create and destroy object
        buzzobj_t object = buzzobj_new(buzzType);
        buzzobj_destroy(&object);

        EXPECT_EQ(object, nullptr);
    }
}

// Test for function `buzzobj_table_hash` defined in "../../src/buzz/buzztype.c"
// Only integers, floats and strings should be usable.
TEST(BuzzObjTableHash, ValidTypes) {
    const uint16_t validBuzzTypes[] = {BUZZTYPE_INT, BUZZTYPE_FLOAT, BUZZTYPE_STRING};

    for (auto &&buzzType : validBuzzTypes) {
        buzzobj_t key = buzzobj_new(buzzType);

        // Verify if valid default value (0) is returned
        EXPECT_EQ(buzzobj_table_hash(&key), 0);

        // Destroy object after usage
        buzzobj_destroy(&key);
    }
}

// Test for function `buzzobj_table_hash` defined in "../../src/buzz/buzztype.c"
// Only integers, floats and strings should be usable.
TEST(BuzzObjTableHash, InvalidTypes) {
    const uint16_t invalidBuzzTypes[] = {BUZZTYPE_NIL, BUZZTYPE_TABLE, BUZZTYPE_CLOSURE, BUZZTYPE_USERDATA};

    for (auto &&buzzType : invalidBuzzTypes) {
        buzzobj_t key = buzzobj_new(buzzType);

        // Verify if function dies with good error message
        EXPECT_DEATH(buzzobj_table_hash(&key), "Can't use a [a-z]* value as table key\n");

        // Destroy object after usage
        buzzobj_destroy(&key);
    }
}

// Test for `buzzobj_hash` defined in <buzz/buzztype.h>
// Hash for nil should always be 0
TEST(BuzzObjHash, Nil) {
    buzzobj_t object = buzzobj_new(BUZZTYPE_NIL);

    EXPECT_EQ(buzzobj_hash(object), 0);

    buzzobj_destroy(&object);
}

// Tests for `buzzobj_hash` defined in <buzz/buzztype.h>
// Hashes for integers should be consistent and unique.
TEST(BuzzObjHash, Int) {
    buzzobj_t objects[3];
    for (auto &&object : objects) {
        object = buzzobj_new(BUZZTYPE_INT);
    }
    objects[2]->f.value = 2;

    // Test hash consistency
    EXPECT_EQ(buzzobj_hash(objects[0]), buzzobj_hash(objects[1]));

    // Test hash unicity
    EXPECT_NE(buzzobj_hash(objects[0]), buzzobj_hash(objects[2]));

    // Destroy objects after usage
    for (auto &&object : objects) {
        buzzobj_destroy(&object);
    }
}

// Tests for `buzzobj_hash` defined in <buzz/buzztype.h>
// Hashes for integers should be consistent and unique.
TEST(BuzzObjHash, Float) {
    buzzobj_t objects[3];
    for (auto &&object : objects) {
        object = buzzobj_new(BUZZTYPE_FLOAT);
    }
    objects[2]->f.value = 2.0f;

    // Test hash consistency
    EXPECT_EQ(buzzobj_hash(objects[0]), buzzobj_hash(objects[1]));

    // Test hash unicity
    EXPECT_NE(buzzobj_hash(objects[0]), buzzobj_hash(objects[2]));

    // Destroy objects after usage
    for (auto &&object : objects) {
        buzzobj_destroy(&object);
    }
}

// Tests for `buzzobj_hash` defined in <buzz/buzztype.h>
// Hashes for strings should be consistent and unique.
TEST(BuzzObjHash, String) {
    buzzobj_t objects[3];
    for (auto &&object : objects) {
        object = buzzobj_new(BUZZTYPE_STRING);
        object->s.value.str = "A string";
    }
    objects[2]->s.value.str = "Another string";

    // Test hash consistency
    EXPECT_EQ(buzzobj_hash(objects[0]), buzzobj_hash(objects[1]));

    // Test hash unicity
    EXPECT_NE(buzzobj_hash(objects[0]), buzzobj_hash(objects[2]));

    // Destroy objects after usage
    for (auto &&object : objects) {
        buzzobj_destroy(&object);
    }
}

// Tests for `buzzobj_hash` defined in <buzz/buzztype.h>
// Hashes for tables should be consistent and unique.
TEST(BuzzObjHash, Table) {
    // Create tables
    const int NB_TABLES = 4;
    buzzobj_t tables[NB_TABLES];
    for (size_t i = 0; i < NB_TABLES; i++) {
        buzzobj_t table = buzzobj_new(BUZZTYPE_TABLE);
        buzzobj_t key = buzzobj_new(BUZZTYPE_INT);
        buzzobj_t value = buzzobj_new(BUZZTYPE_STRING);

        if (i < 2) {
            key->i.value = i;
            value->s.value.str = "A value";
        } else if (i == 2) {
            key->i.value = i;
            value->s.value.str = "Another value";
        } else {
            key->i.value = i + 1;
            value->s.value.str = "A value";
        }

        table->t.value = buzzdict_new(5, sizeof(uint32_t), sizeof(char *), buzzdict_strkeyhash, buzzdict_strkeycmp, NULL);
        buzzdict_set(table->t.value, &key, &value);

        tables[i] = table;
    }

    // Test hash consistency between a table and itself
    EXPECT_EQ(buzzobj_hash(tables[0]), buzzobj_hash(tables[0]));

    // Test hash unicity for tables with identical data
    EXPECT_NE(buzzobj_hash(tables[0]), buzzobj_hash(tables[1]));

    // Test hash unicity for tables with identical keys but different values
    EXPECT_NE(buzzobj_hash(tables[0]), buzzobj_hash(tables[2]));

    // Test hash unicity for tables with diferent keys
    EXPECT_NE(buzzobj_hash(tables[0]), buzzobj_hash(tables[2]));

    // Destroy tables after usage
    for (auto &&table : tables) {
        buzzobj_destroy(&table);
    }
}

// Tests for `buzzobj_hash` defined in <buzz/buzztype.h>
// Hashes for userdata should be consistent and unique.
TEST(BuzzObjHash, UserData) {
    // Create userdata
    const int NB_USERDATA = 3;
    buzzobj_t userdata[NB_USERDATA];
    for (size_t i = 0; i < NB_USERDATA; i++) {
        userdata[i] = buzzobj_new_userdata(buzzobj_new_int(i));
    }

    // Test hash consistency between a userdata and itself
    EXPECT_EQ(buzzobj_hash(userdata[0]), buzzobj_hash(userdata[0]));

    // Test hash unicity for userdata with identical data
    EXPECT_NE(buzzobj_hash(userdata[0]), buzzobj_hash(userdata[1]));

    // Test hash unicity for userdata with different content
    EXPECT_NE(buzzobj_hash(userdata[0]), buzzobj_hash(userdata[2]));

    // Destroy userdata after usage
    for (auto &&userdata : userdata) {
        buzzobj_destroy(&userdata);
    }
}

// Tests for `buzzobj_hash` defined in <buzz/buzztype.h>
// Closures should not be hashable.
TEST(BuzzObjHash, Closure) {
    buzzobj_t closure = buzzobj_new(BUZZTYPE_CLOSURE);

    // Verify if function dies with good error message
    EXPECT_DEATH(buzzobj_hash(closure), "^(\\[BUG\\] )(.*)([0-9]+: Hash for Buzz object type [0-9]+\n)$");

    // Destroy closure after usage
    buzzobj_destroy(&closure);
}

// Tests for `buzzobj_cmp` defined in <buzz/buzztype.h>
// Test cases for comparisons where one or more variable is nil.
TEST(BuzzObjCompare, Nil) {
    // Test for equal value
    EXPECT_EQ(buzzobj_cmp(buzzobj_new(BUZZTYPE_NIL), buzzobj_new(BUZZTYPE_NIL)), 0);

    // Test for nil right value < integer left value
    EXPECT_EQ(buzzobj_cmp(buzzobj_new(BUZZTYPE_NIL), buzzobj_new_int(1)), -1);

    // Test for float right value > nil left value
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_float(-5.6f), buzzobj_new(BUZZTYPE_NIL)), 1);

    // Test for string right value > nil left value
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_string("A string"), buzzobj_new(BUZZTYPE_NIL)), 1);
}

// Tests for `buzzobj_cmp` defined in <buzz/buzztype.h>
// Test cases for comparisons with numeric types.
TEST(BuzzObjCompare, Numeric) {
    // Tests for integer values
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_int(-1), buzzobj_new_int(-1)), 0);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_int(0), buzzobj_new_int(10)), -1);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_int(3), buzzobj_new_int(-65)), 1);

    // Tests for float values
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_float(-1.0f),buzzobj_new_float(-1.0f)), 0);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_float(0.0f), buzzobj_new_float(10.0f)), -1);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_float(3.0f), buzzobj_new_float(-65.0f)), 1);

    // Tests for left integer and right float values
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_int(-1),buzzobj_new_float(-1.0f)), 0);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_int(0), buzzobj_new_float(10.0f)), -1);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_int(3), buzzobj_new_float(-65.0f)), 1);

    // Tests for left float and right integer values
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_float(-1.0f),buzzobj_new_int(-1)), 0);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_float(0.0f), buzzobj_new_int(10)), -1);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_float(3.0f), buzzobj_new_int(-65)), 1);
}

// Tests for `buzzobj_cmp` defined in <buzz/buzztype.h>
// Test cases for comparisons with string types.
TEST(BuzzObjCompare, String) {
    // Tests for string values
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_string("A string"), buzzobj_new_string("A string")), 0);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_string("A string"), buzzobj_new_string("Buzz")), -1);
    EXPECT_EQ(buzzobj_cmp(buzzobj_new_string("Test"), buzzobj_new_string("Buzz")), 1);
}

// Tests for `buzzobj_cmp` defined in <buzz/buzztype.h>
// Test cases for comparisons with incompatible/invalid types.
TEST(BuzzObjCompare, IncompatibleTypes) {
    const char* errorRegex = "^(\\[TODO\\] )(.*)(: Error for comparison between Buzz objects of types [0-9]+ and [0-9]+\n)$";
    
    /* Tests for strings and numeric types. Should die with clear error message */
    // Tests for left string and right integer values
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_string("1"), buzzobj_new_int(1)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_string("100"), buzzobj_new_int(42)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_string("34abc"), buzzobj_new_int(12)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_int(-9), buzzobj_new_string("-9")), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_int(2), buzzobj_new_string("23")), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_int(99), buzzobj_new_string("8a")), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_string("1"), buzzobj_new_int(1)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_string("100"), buzzobj_new_int(42)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_string("34abc"), buzzobj_new_int(12)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_float(-9.0f), buzzobj_new_string("-9.0")), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_float(2.5f), buzzobj_new_string("23")), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_float(99.9f), buzzobj_new_string("8a")), errorRegex);

    /* Tests for closures, should die with clear error message */
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new(BUZZTYPE_CLOSURE), buzzobj_new(BUZZTYPE_CLOSURE)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new(BUZZTYPE_CLOSURE), buzzobj_new_int(1)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_string("3abc0"), buzzobj_new(BUZZTYPE_CLOSURE)), errorRegex);

    /* Tests for comparison between userdata and any other type */
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_string("This is a test string."), buzzobj_new(BUZZTYPE_USERDATA)), errorRegex);
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new_float(87.2f), buzzobj_new(BUZZTYPE_USERDATA)), errorRegex);
    // This comparison might be invalid (userdata should only be compared between themselves) 
    // but does not cause a crash because of sequential cohesion in `buzzobj_cmp`.
    // If a refactor were applied to this method, it could introduce a bug.
    EXPECT_DEATH(buzzobj_cmp(buzzobj_new(BUZZTYPE_USERDATA), buzzobj_new(BUZZTYPE_NIL)), errorRegex);
}
