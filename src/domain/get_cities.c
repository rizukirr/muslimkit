/**
 * @file get_cities.c
 * @brief Implementation of city data fetching and parsing functions.
 *
 * Contains the implementation for making HTTPS requests to retrieve
 * city data from the prayer times API and parsing the JSON responses.
 */

#include "domain/get_cities.h"
#include "lib/json.h"
#include "network/connection.h"

int parse_cities_json(const char *json_str, struct cities_s *dest) {
  JsonNode *root = json_decode(json_str);
  if (root == NULL) {
    printf("parse_cities_json() root NULL\n");
    return -1;
  }

  struct cities_s *result = calloc(1, sizeof(struct cities_s));

  if (result == NULL) {
    json_delete(root);
    printf("parse_cities_json() calloc failure\n");
    return -1;
  }

  const bool status = json_get_bool(root, "status");
  result->status = status;

  JsonNode *data = json_find_member(root, "data");
  if (data && data->tag == JSON_ARRAY) {
    size_t count = 0;
    JsonNode *elem;

    json_foreach(elem, data) { count++; }

    result->data = calloc(count, sizeof(struct cities_data_s));
    if (result->data == NULL) {
      free(result);
      json_delete(root);
      printf("malloc fail");
      return -1;
    }

    result->size = count;

    size_t idx = 0;
    json_foreach(elem, data) {
      JsonNode *id = json_find_member(elem, "id");
      if (id && id->tag == JSON_STRING) {
        char *tmp = id->string_;
        result->data[idx].id = strndup(tmp, strlen(tmp));
      }

      JsonNode *location = json_find_member(elem, "lokasi");
      if (location && location->tag == JSON_STRING) {
        char *tmp = location->string_;
        result->data[idx].lokasi = strndup(tmp, strlen(tmp));
      }

      idx++;
    }
  }

  json_delete(root);
  *dest = *result;
  free(result);
  return 0;
}

int get_city(struct cities_s *dest) {
  int endpoint_len = strlen(API_VERSION) + strlen(CITY_ENDPOINT) + 1;
  char *endpoint = malloc(endpoint_len);
  if (endpoint == NULL) {
    fprintf(stderr, "Cannot allocate memory for get_city endpoint\n");
    return -1;
  }
  snprintf(endpoint, endpoint_len, "%s%s", API_VERSION, CITY_ENDPOINT);

  struct http_response response;
  memset(&response, 0, sizeof(response));

  int get_request = get(HOST, endpoint, &response);
  free(endpoint);

  if (get_request < 0) {
    http_response_free(&response);
    return -1;
  }

  struct cities_s city_s;
  memset(&city_s, 0, sizeof(struct cities_s));
  int city = parse_cities_json(response.body, &city_s);
  if (city < 0) {
    get_city_free(&city_s);
    http_response_free(&response);
    return -1;
  }

  *dest = city_s;
  http_response_free(&response);
  return 0;
}

void get_city_free(struct cities_s *cities) {
  if (cities == NULL) {
    printf("cities NULL\n");
    return;
  }

  for (size_t i = 0; i < cities->size; i++) {
    free(cities->data[i].id);
    free(cities->data[i].lokasi);
  }

  free(cities->data);
}
