#include "domain/get_prayer_times.h"
#include "lib/json.h"
#include "network/connection.h"
#include "utils/tmutils.h"
#include <stdio.h>
#include <string.h>

static int parse_json(char *json_str, struct prayer_times *dest) {
  if (json_str == NULL || dest == NULL) {
    fprintf(stderr, "Cannot parse json response from prayer times\n");
    return -1;
  }

  JsonNode *root = json_decode(json_str);
  if (root == NULL) {
    fprintf(stderr, "Cannot proceed prayer times json\n");
    return -1;
  }

  const bool status = json_get_bool(root, "status");
  dest->status = status;

  JsonNode *request_json = json_find_member(root, "request");
  if (request_json && request_json->tag == JSON_OBJECT) {
    JsonNode *path = json_find_member(request_json, "path");
    if (path && path->tag == JSON_STRING) {

      struct prayer_times_req *request = calloc(1, sizeof(struct prayer_times_req));
      if (request == NULL) {
        fprintf(stderr, "Cannot allocate memory for prayer_tiems_req\n");
        json_delete(root);
        return -1;
      }

      char *tmp = path->string_;
      request->path = strndup(tmp, strlen(tmp));
      dest->req = *request;
      free(request);
    }
  }

  struct prayer_times_data *data = calloc(1, sizeof(struct prayer_times_data));
  if (data == NULL) {
    fprintf(stderr, "Cannot allocate memory for prayer_times_data\n");
    json_delete(root);
    return -1;
  }

  JsonNode *data_json = json_find_member(root, "data");
  if (data_json == NULL) {
    fprintf(stderr, "Cannot find member of 'data'\n");
    free(dest->req.path);
    free(data);
    json_delete(root);
    return -1;
  }

  if (data_json->tag == JSON_OBJECT) {
    JsonNode *id = json_find_member(data_json, "id");
    if (id && id->tag == JSON_NUMBER) {
      data->id = id->number_;
    }

    JsonNode *location = json_find_member(data_json, "lokasi");
    if (location && location->tag == JSON_STRING) {
      char *tmp = location->string_;
      data->location = strndup(tmp, strlen(tmp));
    }

    JsonNode *province = json_find_member(data_json, "daerah");
    if (province && province->tag == JSON_STRING) {
      char *tmp = province->string_;
      data->province = strndup(tmp, strlen(tmp));
    }

    JsonNode *schedules = json_find_member(data_json, "jadwal");
    if (schedules && schedules->tag == JSON_ARRAY) {
      size_t count = 0;
      JsonNode *child;

      json_foreach(child, schedules) { count++; };
      data->schedule_size = (int)count;

      data->schedule = calloc(count, sizeof(struct prayer_times_data_schedule));
      if (data->schedule == NULL) {
        fprintf(stderr, "Cannot allocate memory for prayer times\n");
        free(dest->req.path);
        free(data->location);
        free(data->province);
        free(data);
        json_delete(root);
        return -1;
      }

      size_t index = 0;
      json_foreach(child, schedules) {
        if (child && child->tag == JSON_OBJECT) {

          JsonNode *date = json_find_member(child, "date");
          if (date && date->tag == JSON_STRING) {
            char *tmp = date->string_;
            data->schedule[index].date = strndup(tmp, strlen(tmp));
          }

          JsonNode *fajr = json_find_member(child, "subuh");
          if (fajr && fajr->tag == JSON_STRING) {
            char *tmp = fajr->string_;
            data->schedule[index].fajr = strndup(tmp, strlen(tmp));
          }

          JsonNode *dhuha = json_find_member(child, "dhuha");
          if (dhuha && dhuha->tag == JSON_STRING) {
            char *tmp = dhuha->string_;
            data->schedule[index].dhuha = strndup(tmp, strlen(tmp));
          }

          JsonNode *dzuhr = json_find_member(child, "dzuhur");
          if (dzuhr && dzuhr->tag == JSON_STRING) {
            char *tmp = dzuhr->string_;
            data->schedule[index].dzuhr = strndup(tmp, strlen(tmp));
          }

          JsonNode *ashr = json_find_member(child, "ashar");
          if (ashr && ashr->tag == JSON_STRING) {
            char *tmp = ashr->string_;
            data->schedule[index].ashr = strndup(tmp, strlen(tmp));
          }

          JsonNode *maghrib = json_find_member(child, "maghrib");
          if (maghrib && maghrib->tag == JSON_STRING) {
            char *tmp = maghrib->string_;
            data->schedule[index].maghrib = strndup(tmp, strlen(tmp));
          }

          JsonNode *isya = json_find_member(child, "isya");
          if (isya && isya->tag == JSON_STRING) {
            char *tmp = isya->string_;
            data->schedule[index].isya = strndup(tmp, strlen(tmp));
          }
        }
        index++;
      }
    }
  }

  dest->data = *data;
  free(data);
  json_delete(root);
  return 0;
}

int get_prayer_times(const char *city_id, struct prayer_times *dest) {
  if (city_id == NULL || dest == NULL) {
    fprintf(stderr, "City id or destination must be not NULL\n");
    return -1;
  }

  struct tmutils tm;
  memset(&tm, 0, sizeof(tm));
  get_current_time(&tm);

  int year = tm.year;
  int month = tm.month;
  // /idid/yyyy/mm + null terminator
  char path[14];
  snprintf(path, 14, "/%s/%d/%d", city_id, year, month);

  int endpoint_len = strlen(API_VERSION) + strlen(PRAYER_TIME_ENDPOINT) + strlen(path) + 1;

  char *endpoint = malloc(endpoint_len);
  if (endpoint == NULL) {
    fprintf(stderr, "Cannot allocate memory for get prayer times\n");
    return -1;
  }

  snprintf(endpoint, endpoint_len, "%s%s%s", API_VERSION, PRAYER_TIME_ENDPOINT, path);

  struct http_response response;
  memset(&response, 0, sizeof(response));
  int get_request = get(HOST, endpoint, &response);
  if (get_request < 0) {
    fprintf(stderr, "GET prayer times faile\n");
    free(endpoint);
    return -1;
  }
  free(endpoint);

  struct prayer_times prayer_t;
  memset(&prayer_t, 0, sizeof(prayer_t));

  int parse = parse_json(response.body, &prayer_t);
  if (parse < 0) {
    fprintf(stderr, "Parse json fail\n");
    get_prayer_times_free(&prayer_t);
    http_response_free(&response);
    return -1;
  }

  *dest = prayer_t;
  http_response_free(&response);
  return 0;
}

void get_prayer_times_free(struct prayer_times *prayer_t) {
  if (prayer_t == NULL) {
    return;
  }

  free(prayer_t->req.path);
  free(prayer_t->data.location);
  free(prayer_t->data.province);

  if (prayer_t->data.schedule_size > 0) {
    for (int i = 0; i < prayer_t->data.schedule_size; i++) {
      free(prayer_t->data.schedule[i].date);
      free(prayer_t->data.schedule[i].fajr);
      free(prayer_t->data.schedule[i].dhuha);
      free(prayer_t->data.schedule[i].dzuhr);
      free(prayer_t->data.schedule[i].ashr);
      free(prayer_t->data.schedule[i].maghrib);
      free(prayer_t->data.schedule[i].isya);
    }
    free(prayer_t->data.schedule);
  }
}
