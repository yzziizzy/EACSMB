


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
	else if(0 == strcmp("decal", name)) {
		return ITEM_TYPE_DECAL;
	}
	else if(0 == strcmp("marker", name)) {
		return ITEM_TYPE_MARKER;
	}
	else if(0 == strcmp("item", name)) {
		return ITEM_TYPE_ITEM;
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

		case ITEM_TYPE_DECAL:
			part->index = DecalManager_lookupName(w->dm, name);
			break;
			
		case ITEM_TYPE_MARKER:
			part->index = MarkerManager_lookupName(w->mm, name);
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


///////////////// new universal code /////////////////



static int add_part(World* w, Part p) {
	
	// TODO: check for name collisions
	
	
	int pi = VEC_LEN(&w->parts);
	VEC_PUSH(&w->parts, p);
	if(HT_set(&w->partLookup, p.name, pi)) {
		fprintf(stderr, "failed to register part '%s'\n", p.name);
	}

	return pi;
}




// returns part index
static int loadConfig_DynamicMesh(World* w, json_value_t* jo);
static int loadConfig_Emitter(World* w, json_value_t* jo);
static int loadConfig_Light(World* w, json_value_t* jo);
static int loadConfig_Decal(World* w, json_value_t* jo);
static int loadConfig_CustomDecal(World* w, json_value_t* jo);
static int loadConfig_Marker(World* w, json_value_t* jo);


typedef int (*loaderFn)(World*, json_value_t*);

static const loaderFn loaderFns[] = {
	[ITEM_TYPE_UNKNOWN] =     NULL,
	[ITEM_TYPE_STATICMESH] =  NULL, // obsolete, for now
	[ITEM_TYPE_DYNAMICMESH] = loadConfig_DynamicMesh,
	[ITEM_TYPE_EMITTER] =     loadConfig_Emitter,
	[ITEM_TYPE_LIGHT] =       loadConfig_Light,
	[ITEM_TYPE_DECAL] =       loadConfig_Decal,
	[ITEM_TYPE_CUSTOMDECAL] = loadConfig_CustomDecal,
	[ITEM_TYPE_MARKER] =      loadConfig_Marker,
};




void World_loadItemConfigFileNew(World* w, char* path) {
	json_file_t* jsf;
		
	jsf = json_load_path(path);
	World_loadItemConfigNew(w, jsf->root);
}


void World_loadItemConfigNew(World* w, json_value_t* jo) {
	
	
	if(jo->type == JSON_TYPE_ARRAY) {
		struct json_array_node* link;
		
		link = jo->v.arr->head;
		while(link) {
			loaderFn fn;
			enum ItemTypes type;
			char* tname;
			json_value_t* val;
			
			if(link->value->type != JSON_TYPE_OBJ) {
				printf("invalid item format\n");
				
				link = link->next;
				continue;
			}
			
			// sniff the type
			int ret = json_obj_get_key(link->value, "_type", &val);
			json_as_string(val, &tname);
			
			type = partTypeLookup(tname);
		
			
			fn = loaderFns[type];
			if(fn) {
				fn(w, link->value);
			}
			else {
				printf("failed to parse item/part definition\n");
			}
			
			link = link->next;
		}
		
		
	}
	else if(jo->type == JSON_TYPE_OBJ) {
		
	}
	
}





// returns part index
static int loadConfig_DynamicMesh(World* w, json_value_t* jo) {
	
	json_value_t* val;
	char* path;
	DynamicMesh* dm;
	
	OBJContents obj;
	
	char* name = json_obj_key_as_string(jo, "_name");
	
	int ret = json_obj_get_key(jo, "mesh", &val);
	json_as_string(val, &path);
	
	// TODO: fix
	loadOBJFile(path, 0, &obj);
	dm = DynamicMeshFromOBJ(&obj);
	dm->name = name;
	
	
	ret = json_obj_get_key(jo, "texture", &val);
	if(!ret) {
		json_as_string(val, &path);
		
		dm->texIndex = TextureManager_reservePath(w->dmm->tm, path);
		printf("dmm: %d %s\n", dm->texIndex, path);
	}

#define grab_json_val(str, field, def) \
	dm->field = def; \
	if(!json_obj_get_key(jo, str, &val)) { \
		json_as_float(val, &dm->field); \
	}

	grab_json_val("scale", defaultScale, 1.0)
	grab_json_val("rotDegX", defaultRotX, 0.0)
	grab_json_val("rotDegY", defaultRotY, 0.0)
	grab_json_val("rotDegZ", defaultRotZ, 0.0)
	
	// radians are not easy to edit in a config file, so it's in degrees
	dm->defaultRotX *= F_PI / 180.0;  
	dm->defaultRotY *= F_PI / 180.0;  
	dm->defaultRotZ *= F_PI / 180.0;  
	

	int ind = dynamicMeshManager_addMesh(w->dmm, dm->name, dm);
	printf("DM added mesh %d: %s \n", ind, dm->name);
	
	
	return add_part(w, (Part){ITEM_TYPE_DYNAMICMESH, ind, name});
}






// returns part index
static int loadConfig_Emitter(World* w, json_value_t* jo) {

}


// returns part index
static int loadConfig_Light(World* w, json_value_t* jo) {
	
	
}


// returns part index
static int loadConfig_Decal(World* w, json_value_t* jo) {
// 	int ret;
// 	struct json_obj* o;
// 	void* iter;
// 	char* key, *texName, *tmp;
// 	struct json_value* v, *tc;
// 	json_file_t* jsf;
	
	


	json_value_t* val;
	char* path;
	Decal* d;
	
	//OBJContents obj;
	
	//ret = json_obj_get_key(tc, "mesh", &val);
	//json_as_string(val, &path);
	
	//loadOBJFile(path, 0, &obj);
	//d = DynamicMeshFromOBJ(&obj);
	pcalloc(d);
	//d->name = strdup(key);
	
	
	int ret = json_obj_get_key(jo, "texture", &val);
	if(!ret) {
		json_as_string(val, &path);
		
		d->texIndex = TextureManager_reservePath(w->dmm->tm, path);
		printf("dm: %d %s\n", d->texIndex, path);
	}

#define grab_json_val(str, field, def) \
	d->field = def; \
	if(!json_obj_get_key(jo, str, &val)) { \
		json_as_float(val, &d->field); \
	}

	grab_json_val("scale", size, 1.0)
	

	int ind = DecalManager_AddDecal(w->dmm, d->name, d);
	printf("DM added decal %d: %s \n", ind, d->name);
	
	
	// save name
	char* name = json_obj_key_as_string(jo, "_name");
	json_as_string(val, &name);
	
	return add_part(w, (Part){ITEM_TYPE_DECAL, ind, name});
}


// returns part index
static int loadConfig_CustomDecal(World* w, json_value_t* jo) {
	
	
}


// returns part index
static int loadConfig_Marker(World* w, json_value_t* jo) {
	
	
}






