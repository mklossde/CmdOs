
//------------------------------------------------------------
// Attr

MapList attrMap(true); 

/* remove '$' from key */
char* _toAttr(char *key) {
  if(!is(key)) { return EMPTY; } else if(*key=='$') { key++; }
  return key;
}

boolean attrHave(char *key) { return attrMap.find(_toAttr(key))!=-1; }
char* attrGet(char *key) {  return (char*)attrMap.get(_toAttr(key)); }
void attrSet(char *key,String value) { 
  char *v=(char*)value.c_str(); 
  attrMap.replace(_toAttr(key),v,strlen(v)); 
} 
void attrSet(char *key,char *value) { attrMap.replace(_toAttr(key),value,strlen(value));  }
void attrDel(char *key) { attrMap.del(_toAttr(key)); }
char* attrInfo() {
  sprintf(buffer,"attrs:\n");
  for(int i=0;i<attrMap.size();i++) {  
    char *key=attrMap.key(i);
    char *value=(char*)attrMap.get(i);
    if(is(key) && is(value)) {
      sprintf(buffer+strlen(buffer),"attr %s \"%s\"\n",key,value);
    }
  }
  return buffer;
}
void attrClear(char *prefix) { attrMap.clear(prefix); }

//------------------------------------------------------------
// Sys

/* get sys attribute */
char* sysAttr(char *name) {
  int d=0; char* s;
  if(!is(name)) { return EMPTY; }
  else if(equals(name,"timestamp")) { d=_timeMs;  }
  else if(equals(name,"time")) { s=getTime();  }
  else if(equals(name,"date")) { s=getDate(); }
  else if(equals(name,"ip")) { s=(char*)appIP.c_str(); }
  else if(equals(name, "freeHeap")) { d=ESP.getFreeHeap(); }// show free heap
  else if(equals(name, "freeHeapMax")) { d=freeHeapMax; }// show free heap  
  else { return EMPTY; }

  if(is(s)) { sprintf(paramBuffer,"%s",s); } else { sprintf(paramBuffer,"%d",d); }
  return paramBuffer;   
}

//------------------------------------------------------------
// Params


MapList appParams(true);

class AppParam {
  private:
    char *_name;
    char *_remote;
  public:
    boolean _change=false;
    byte _type=0;

    char* name() { return _name; }
    char* remote() { return _remote; }
    boolean is(char *remote) { return equals(remote,_remote); }
    char* get() { return (char*)attrGet(_name); }
    void set(char *value) { attrSet(_name,value);_change=true; }

    AppParam(char *name,char *remote,byte type) { 
        _name=copy(name); _remote=copy(remote);
        _type=type;
    }
    ~AppParam() { free(_name);  free(_remote); }
};

//------

char* paramsInfo() {
  sprintf(buffer,"params:\n");
  for(int i=0;i<appParams.size();i++) {      
    char *key=appParams.key(i);
    AppParam *param=(AppParam*)appParams.get(i);
    sprintf(buffer+strlen(buffer),"param %s remote:%s type:%d change:%d \n",param->name(),param->remote(),param->_type,param->_change);
  }
  return buffer;
}

boolean paramSet(char *remote,char *value) {
    for(int i=0;i<appParams.size();i++) {  
    AppParam *p=(AppParam*)appParams.get(i);
    if(p->is(remote)) { 
      p->set(value);
      return true;
    }
  }
  return false;
}

int paramsFind(char *remote) {
  for(int i=0;i<appParams.size();i++) {  
    AppParam *p=(AppParam*)appParams.get(i);
    if(p->is(remote)) { return i; }
  }
  return NULL;
}

void* paramsGet(char *name) {
  return appParams.get(name);
}

//------

char* paramRemote(char *name) {
  AppParam *param=(AppParam*)appParams.get(name);
  if(param!=NULL) { return param->remote(); } else { return NULL; }
}

boolean paramsDel(char *name) {
  AppParam *param=(AppParam*)appParams.get(name);
  if(param==NULL) { return false; }
  appParams.del(name);
  return true;
}


boolean paramsAdd(char *name,char *remote,byte type) {
  AppParam *p=new AppParam(name,remote,type);
  appParams.set(name,p);  // add new param
  return true;
}

/*
boolean paramsAdd(AppParam *p) {
  if(paramGet(p->name)!=NULL) { return false; }
  appParams.add(p->name(),p);  // add new param
  return true;
}
*/
