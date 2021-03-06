#include "stdafx.h"
#include <assert.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "data_variables.h"

static void add_fake_data(mongoc_collection_t *collection)
{
	mongoc_bulk_operation_t *bulk;
	bson_error_t error;
	bson_t *doc;
	bson_t reply;
	time_t t;
	bool ret;
	printf("Generating documents... \n");
	//printf("%s\n", name_array[1]);
	bulk = mongoc_collection_create_bulk_operation(collection, true, NULL);
	// take the first instance of the array to do the calculations
	// not forgetting to take one for the size of the index
	int size_of_first_names_array = (sizeof(first_names_array) / sizeof(first_names_array[0])) - 1;
	int size_of_city_array = (sizeof(city_array) / sizeof(city_array[0])) - 1;
	int size_of_last_names_array = (sizeof(last_names_array) / sizeof(last_names_array[0])) - 1;
	/* Intializes random number generator */
	srand((unsigned)time(&t));
	int i;
	for (i = 0; i < 100000; i++) {
		int age = rand() % 100;
		int newage = rand() % 3;
		// generate first_name, last name, age and city
		char *first_name = first_names_array[rand() % size_of_first_names_array];
		char *last_name = last_names_array[rand() % size_of_last_names_array];
		char *city = city_array[rand() % size_of_city_array];
		doc = BCON_NEW("first_name", first_name);
		BCON_APPEND(doc, "age", BCON_INT32(age));
		BCON_APPEND(doc, "last_name", last_name);
		BCON_APPEND(doc, "city", city);
		// insert to bulk
		mongoc_bulk_operation_insert(bulk, doc);
		// destroy each doc
		bson_destroy(doc);
	}

	// get return value
	ret = mongoc_bulk_operation_execute(bulk, &reply, &error);
	// convert bson to json
	char *str = bson_as_json(&reply, NULL);
	printf("%s\n", str);
	// deallocate bson
	bson_free(str);

	if (!ret) {
		fprintf(stderr, "Error: %s\n", error.message);
	}
	// destroy mongo reply
	bson_destroy(&reply);
	// destroy bulk
	mongoc_bulk_operation_destroy(bulk);
}

int main(int argc, char *argv[])
{
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	// initialize the mongo client
	mongoc_init();

	// connect to the mongo instance
	client = mongoc_client_new("mongodb://localhost/");
	if (argc == 3)
	{
		// get parameters one and two
		char **database_name = argv[1];
		char **collection_name = argv[2];
		// connect to the test collection
		collection = mongoc_client_get_collection(client, database_name, collection_name);
	}
	else
	{
		printf("No parameters given so assume TestDB and test collection... \n");
		// connect to the test collection as no parameter given
		collection = mongoc_client_get_collection(client, "test", "test");
	}

	// call the fake data function
	add_fake_data(collection);
	// cleanup the connection
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(client);
	mongoc_cleanup();
	// return 0 for success
	return 0;
}