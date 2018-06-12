


#include <string.h>

#include "world.h"
#include "c_json/json.h"
#include "json_gl.h"


int partTypeLookup(char* name) {
	if(0 == strcmp("staticMesh", name)) {
		return ITEM_TYPE_STATICMESH;
	}
	else if(0 == strcmp("dynamicMesh", name)) {
		return ITEM_TYPE_DYNAMICMESH;
	}
	else if(0 == strcmp("emitter", name)) {
		return ITEM_TYPE_EMITTER;
	}
	else if(0 == strcmp("light", name)) {
		return ITEM_TYPE_LIGHT;
	}
	else {
		printf("Unknown part type: %s\n", name);
		return ITEM_TYPE_UNKNOWN;
	}
}



ItemPart* findPart(World* w, char* typeName, char* name, ItemPart* part) {
	
	part->type = partTypeLookup(typeName);
	printf("type: %s %d\n", typeName, part->type);
	switch(part->type) {
		case ITEM_TYPE_DYNAMICMESH:
			part->index = dynamicMeshManager_lookupName(w->dmm, name);
			break;
			
		case ITEM_TYPE_STATICMESH:
			part->index = meshManager_lookupName(w->smm, name);
			break;
			
		case ITEM_TYPE_EMITTER:
			printf("!!! NYI: lookup of emitter instance for ItemParts\n");
			//part->index = meshManager_lookupName(w->smm, name);
			part->index = 0;
			break;
			
		default:
			free(part);
			break;
	}
	
	return part;
}






void loadItemConfig(World* w, char* path) {
	
	void* item_iter;
	char* itemName;
	json_value_t* j_item, *j_parts;
	json_file_t* jsf;
	
	jsf = json_load_path("assets/config/items.json");
	
	json_value_t* tex;
	json_obj_get_key(jsf->root, "textures", &tex);
	
	
	item_iter = NULL;
	while(json_obj_next(jsf->root, &item_iter, &itemName, &j_item)) {
		Item* item;
		int len, i;
		json_array_node_t* j_part_node;
		
		item = calloc(1, sizeof(*item));
		item->name = strdup(itemName);
		
		printf("-item '%s'\n", itemName);
		
		json_obj_get_key(j_item, "parts", &j_parts);
		
		len = json_array_length(j_parts);
		item->numParts = len;
		item->parts = calloc(1, len * sizeof(*item->parts));
		
		// collect up all the parts
		j_part_node = j_parts->v.arr->head;
		for(i = 0; i < len && j_part_node; i++) {
			json_value_t* v, *j_part;
			char* type, *name;
			
			j_part = j_part_node->value;
			
			json_obj_get_key(j_part, "type", &v);
			json_as_string(v, &type);
			
			if(0 == strcmp("light", type)) {
				item->parts[i].type = ITEM_TYPE_LIGHT;
				
				json_obj_get_key(j_part, "position", &v);
				json_as_vector(v, 3, &item->parts[i].offset);
				
				//json_obj_get_key(j_part, "lightType", &v);
				//json_as_string(v, &lightType);
				float intensity;
				json_obj_get_key(j_part, "intensity", &v);
				json_as_float(v, &intensity);
				
			}
			else {			
				json_obj_get_key(j_part, "name", &v);
				json_as_string(v, &name);
				
				printf("  `-found part '%s'\n", name);
				
				findPart(w, type, name, &item->parts[i]); 
				printf("    `-type: %d\n", item->parts[i].type);
				printf("    `-model num: %d\n", item->parts[i].index);
				
				json_obj_get_key(j_part, "position", &v);
				json_as_vector(v, 3, &item->parts[i].offset);
			}
			
			j_part_node = j_part_node->next;
		}

		// add item to the list
		VEC_PUSH(&w->items, item);
		if(HT_set(&w->itemLookup, item->name, VEC_LEN(&w->items) - 1)) {
 			fprintf(stderr, "failed to register item '%s'\n", item->name);
 		}
	}
	
	
}























