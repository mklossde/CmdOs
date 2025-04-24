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
  sprintf(buffer,"");
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
  else { return EMPTY; }

  if(is(s)) { sprintf(paramBuffer,"%s",s); } else { sprintf(paramBuffer,"%d",d); }
  return paramBuffer;   
}
