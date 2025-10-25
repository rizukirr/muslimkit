/**
 * @file get_cities.c
 * @brief Implementation of city data fetching and parsing functions.
 *
 * Contains the implementation for making HTTPS requests to retrieve
 * city data from the prayer times API and parsing the JSON responses.
 */

#include "domain/get_cities.h"
#include "network/connection.h"
#include "lib/json.h"

struct cities_s *parse_cities_json(const char *json_str) {
  JsonNode *root = json_decode(json_str);
  if (root == NULL) {
    json_delete(root);
    printf("parse_cities_json() root NULL\n");
    return NULL;
  }

  struct cities_s *result = calloc(1, sizeof(struct cities_s));

  if (result == NULL) {
    json_delete(root);
    printf("parse_cities_json() calloc failure\n");
    return NULL;
  }

  const bool status = json_get_bool(root, "status");
  result->status = status;

  JsonNode *data = json_find_member(root, "data");
  if (data && data->tag == JSON_ARRAY) {
    size_t count = 0;
    JsonNode *elem;

    json_foreach(elem, data) { count++; }

    result->data = malloc(count * sizeof(struct cities_data_s));
    if (result->data == NULL) {
      free(result->data);
      free(result);
      json_delete(root);
      printf("malloc fail");
      return NULL;
    }

    result->size = count;

    size_t idx = 0;
    json_foreach(elem, data) {
      const char *id = json_get_string(elem, "id");

      if (id == NULL) {
        printf("id NULL\n");
        free(result->data);
        free(result);
        json_delete(root);
        return NULL;
      }

      const char *location = json_get_string(elem, "lokasi");
      if (location == NULL) {
        free(result->data);
        free(result);
        json_delete(root);
        printf("location NULL\n");
        return NULL;
      }

      result->data[idx].id = strndup(id, strlen(id));
      result->data[idx].lokasi = strndup(location, strlen(location));

      idx++;
    }
  }

  json_delete(root);
  return result;
}

struct cities_s *get_city() {
  int endpoint_len = strlen(API_VERSION) + strlen(CITY_ENDPOINT) + 1;
  char *endpoint = malloc(endpoint_len);
  snprintf(endpoint, endpoint_len, "%s%s", API_VERSION, CITY_ENDPOINT);

  struct http_response response;
  memset(&response, 0, sizeof(response));

  int get_request = get(HOST, endpoint, &response);
  free(endpoint);

  if (get_request < 0) {
    http_response_free(&response);
    return NULL;
  }

  struct cities_s *city = parse_cities_json(response.body);
  if (city == NULL) {
    get_city_free(city);
    http_response_free(&response);
    return NULL;
  }

  http_response_free(&response);
  return city;
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
  free(cities);
}
