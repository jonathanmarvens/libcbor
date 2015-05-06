/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "maps.h"

size_t cbor_map_size(const cbor_item_t *item)
{
	assert(cbor_isa_map(item));
	return item->metadata.map_metadata.size;
}

cbor_item_t *cbor_new_definite_map(size_t size)
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	if (item == NULL)
		return NULL;
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_MAP,
		.metadata = {.map_metadata = {
			.size = size,
			.type = _CBOR_METADATA_DEFINITE,
			.ptr = 0
		}},
		.data = _CBOR_MALLOC(sizeof(struct cbor_pair) * size)
	};
	if (item->data == NULL) {
		_CBOR_FREE(item);
		return NULL;
	}
	return item;
}

cbor_item_t *cbor_new_indefinite_map()
{
	cbor_item_t *item = _CBOR_MALLOC(sizeof(cbor_item_t));
	if (item == NULL)
		return NULL;
	*item = (cbor_item_t) {
		.refcount = 1,
		.type = CBOR_TYPE_MAP,
		.metadata = {.map_metadata = {
			.size = 0,
			.type = _CBOR_METADATA_INDEFINITE,
			.ptr = 0
		}},
		.data = NULL
	};
	if (item->data == NULL) {
		_CBOR_FREE(item);
		return NULL;
	}
	return item;
}

cbor_item_t *cbor_map_add(cbor_item_t *item, struct cbor_pair pair)
{
	assert(cbor_isa_map(item));
	cbor_incref(pair.value);
	cbor_map_add_key(item, pair.key);
	// TODO propagate realloc fail
	cbor_map_handle(item)[
		item->metadata.map_metadata.ptr - 1
	].value = pair.value;
	return item;
}

cbor_item_t *cbor_map_add_value(cbor_item_t *item, cbor_item_t *value)
{
	assert(cbor_isa_map(item));
	cbor_incref(value);
	// TODO propagate realloc fail
	cbor_map_handle(item)[
		item->metadata.map_metadata.ptr - 1
	].value = value;
	return item;
}

cbor_item_t *cbor_map_add_key(cbor_item_t *item, cbor_item_t *key)
{
	assert(cbor_isa_map(item));
	cbor_incref(key);
	struct _cbor_map_metadata *metadata = (struct _cbor_map_metadata *) &item->metadata;
	struct cbor_pair *data = cbor_map_handle(item);
	if (cbor_map_is_definite(item)) {
		if (metadata->ptr > metadata->size)
			printf("Error - NE space def\n");
		data[metadata->ptr].key = key;
		data[metadata->ptr++].value = NULL;
	} else {
		// TODO exponential reallocs?
		if (metadata->ptr > metadata->size)
			printf("Error - NE space indef\n");
		data = _CBOR_REALLOC(data, (++metadata->size) * sizeof(struct cbor_pair));
		data[metadata->ptr].key = key;
		data[metadata->ptr++].value = NULL;
		item->data = (unsigned char *) data;
	}
	return item;
}

bool cbor_map_is_definite(const cbor_item_t *item)
{
	assert(cbor_isa_map(item));
	return item->metadata.map_metadata.type == _CBOR_METADATA_DEFINITE;
}

bool cbor_map_is_indefinite(const cbor_item_t *item)
{
	return !cbor_map_is_definite(item);
}

struct cbor_pair *cbor_map_handle(const cbor_item_t *item)
{
	assert(cbor_isa_map(item));
	return (struct cbor_pair *) item->data;
}