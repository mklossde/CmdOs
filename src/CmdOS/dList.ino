

//-------------------------------------------------------------------------------------------------------------------
//  MapList



/* list of object and map of key=value */
class MapList {
private:
  int _index=0; int _max=0;
  void** _array=NULL; // contains values 
  char** _key=NULL; // contains keys
  int* _vsize=NULL; // contains alloc-size of each value
  boolean _isMap=false;

  void grow(int grow) {
    _max+=grow; 
    if(_array==NULL) {          
//      _array = (void**) malloc(_max * sizeof(void*));
      _array = (void**)malloc(_max * sizeof(void*));
      if(_isMap) { _key = (char**) malloc(_max * sizeof(char*)); }
      _vsize = (int*) malloc(_max * sizeof(int));
  }else {
      _array = (void**)realloc(_array, _max * sizeof(void*)); 
      if(_isMap) { _key = (char**)realloc(_key, _max * sizeof(char*)); }    
      _vsize = (int*)realloc(_vsize, _max * sizeof(int));
    }
  }
  void growTo(int max,void *obj) {
    for(int i=_max;i<max;i++) { grow(1);_array[_max-1]=obj; }
  }

public:
  // map
  /* set by copy key and value and replace value on change */
  void replace(char *key,char *obj,int len) {
    if(!is(key)) { return ; } 
    int index=find(key);
    if(index==-1) {
      if(_index>=_max) { grow(1); }       
      int size=len; if(size<minValueLen) { size=minValueLen; }
      char* to=new char[size+1]; if(to==NULL) { espRestart("replace() memory error"); }
      if(len>0) { memcpy( to, obj, len); } 
      to[len]='\0'; 
      _key[_index]=copy(key); _array[_index]=to; _vsize[_index]=size;      
      sprintf(buffer,"set %d '%s'='%s' len:%d size:%d",_index,key,to,len,size); logPrintln(LOG_DEBUG,buffer);
      _index++;
    }else {
      void* old=(void*)_array[index];
      int oldSize=_vsize[index];
      if(oldSize<=len) {
        _array[index] = (void*)realloc(_array[index], len+1); if( _array[index]==NULL) { espRestart("map-replace memory error"); }
        _vsize[index]=len;        
      }         
      char* o=(char*)_array[index]; 
      if(len>0) { memcpy(o, obj, len); } o[len]='\0';
      sprintf(buffer,"replace '%s'='%s' len:%d oldSize:%d",key,o,len,oldSize); logPrintln(LOG_DEBUG,buffer);
    }
  }
  
  /* set key=value into list e.g. list.set("key",value); */
  void* set(char *key,void *obj) {  
    int index=find(key);   
    if(index>=0) { void* old=_array[index]; _array[index]=obj; return old; } // overwrite 
    else {
      if(_index>=_max) { grow(1); } 
      char *k=copy(key); _key[_index]=k; _array[_index]=obj; _index++;        
      return NULL;
    }
  }  
  /* get key at index e.g. char *key=list.key(0); */
  char* key(int index) { if(index>=0 && index<_index) { return _key[index]; } else { return NULL; } }  
  /* get value with key e.g. char *value=(char*)list.get(key); */
  void* get(char *key) {  
    if(!is(key)) { return NULL; }
    for(int i=0;i<_index;i++) {  if(equals(_key[i],key)) { return _array[i]; } } 
    return NULL;
  } 
  /* del key=value e.g. char* old=list.del(key); */
  boolean del(char *key) { 
    if(!is(key)) { return false; }
    int index=find(key); if(index==-1) { return false; }
    del(index);        
    return true;
  }  
  /* find index of key e.g. int index=list.find(key); */
  int find(char *key) { 
    if(!is(key)) { return -1; }
    for(int i=0;i<_index;i++) {  if(equals(_key[i],key)) { return i; } } return -1; }

  // list ------------------------------------------------
  /* add object to list e.g. list.add(obj); */
  void add(void *obj) { if(_index>=_max) { grow(1); } _array[_index++]=obj; } 
  void addIndex(int index,void *obj) { 
    if(index>=_max) { growTo(index+1,NULL); }     
    _array[index]=obj; if(index>=_index) { _index=index+1; } } 
  /* get obejct at index e.g. char* value=(char*)list.get(0); */
  void* get(int index) { if(index>=0 && index<_index) { return _array[index]; } else { return NULL; } }  
  /* del object at index e.g. char* old=(char*)list.del(0); */
  boolean del(int index) {   
    if(index<0 || index>=_index) {return false; }      
    void *obj=_array[index]; if(obj!=NULL) { delete obj; } 
    if(_isMap) { void *oldKey=_key[index]; if(oldKey!=NULL) { delete oldKey; } }        
    for(int i=index;i<_index-1;i++) { 
      _array[i]=_array[i+1]; 
      if(_isMap) { _key[i]=_key[i+1]; }
      _vsize[i]=_vsize[i+1];
    } 
    _index--; 
    return true;    
  }
  /* clear all (without prefix) / clear with prefix (e.g. clear my ) */
  void clear(char *prefix) { for(int i=_index;i>=0;i--) { if(!is(prefix) || startWith(key(i),prefix)) { del(i); }} }

  /* size of list e.g. int size=list.size(); */
  int size() { return _index; }
  MapList(int max) {  grow(max); }
  MapList() {   }
  MapList(boolean isMap) {  _isMap=isMap;  } // enable as map
  ~MapList() { delete _array; if(_isMap) { delete _key; } }

};

