
//-----------------------------------------------------------------------------
// Memory

// new [] => delete[]
// NEW => delete
// malloc() or calloc() => free

char* newChar(int len) {
  return new char[len];
}

void* newMalloc(int size) {
  return (void*)malloc(size);
}

/* buffered new char of len , by use oldchar (buffer first:\0 end of char, second:\0 end of buffer) */
char* newChar(char *oldChar,int len) {
  if(oldChar!=NULL) { 
    int blen=newCharLen(oldChar);
    if(blen>len) { 
      for(int i=0;i<blen-1;i++) { oldChar[i]=' ';}  oldChar[len-1]='\0';  
      return oldChar; }
    else { delete oldChar; }
  }
  int size=len; if(len<minValueLen) { size=minValueLen; }
  char *newChar= new char[size+1]; 
  for(int i=0;i<size-1;i++) { newChar[i]=' ';} newChar[len-1]='\0'; newChar[size]='\0'; // double \0
  return newChar;
}

/* buffered copy char (buffer first:\0 end of char, second:\0 end of buffer) */
char* newChar(char *oldChar,char *str) {
  if(str==NULL) { if(oldChar!=NULL) { oldChar[0]='\0'; } return oldChar; }
  int len=strlen(str);
  oldChar=newChar(oldChar,len+1);
  memcpy(oldChar, str, len); //to[len]='\0';   
  return oldChar;
}  

/* buffered char len */
int newCharLen(char *oldChar) {
  if(oldChar==NULL) { return 0; }
  byte count=0; int blen=0; while(count<2) { if(oldChar[blen++]=='\0') { count++; } }
  return blen; 
}

//-----------------------------------------------------------------------------
// dynamic memory

/* concat char to new char*, use NULL as END, (e.g char *res=concat("one","two",NULL); ), dont forget to free(res); */
char* concat(char* first, ...) {
    size_t total_len = 0;

    va_list args;
    va_start(args, first);
    size_t l=0;
    for (char* s = first; s != NULL && (l=strlen(s))>0; s = va_arg(args, char*)) {  total_len += l;  }
    va_end(args);

    char *result = (char*)malloc(sizeof(char) *(total_len + 1)); // +1 for null terminator
    if (!result) return NULL;
    result[0] = '\0'; // initialize empty string

    va_start(args, first);
    for (char* s = first; s != NULL; s = va_arg(args, char*)) { strcat(result, s); }
    va_end(args);
    return result;
}

/* copy org* to new (NEW CHAR[]
    e.g. char* n=copy(old); 
*/
char* copy(char* org) { 
  if(org==NULL) { return NULL; }
  int len=strlen(org);
  char* newStr=newChar(len+1); 
  memcpy( newStr, org, len); newStr[len]='\0'; 
  return newStr;
}

/* create a copy of org with new char[max] (NEW CHAR[])*/
char* copy(char *to,char* org,int max) { 
  if(to==NULL) { to=newChar(max+1); }
  if(to==NULL) { espRestart("copy() memory error"); }
  if(org!=NULL) { 
    int len=strlen(org); if(len>max) { len=max; }
    memcpy( to, org, len); to[len]='\0'; 
  }else { to[0]='\0'; }
  return to;
}

/* copy (MALLOC) */
char* copy(char *to,String str,int max) { 
  if(to==NULL) { to = (char*)malloc((max + 1)*sizeof(char));  }     
  if(to==NULL) { espRestart("copy() memory error"); }
  if(str!=NULL) {         
//TODO take care on string len    
    strcpy(to, str.c_str()); 
  }
  return to;
}

char* copy(String str) {  
  if(str==NULL || str==EMPTYSTRING) { return NULL; } 
  char* s = (char*)malloc(str.length() + 1); 
  if(s==NULL) { espRestart("to() memory error"); }
  strcpy(s, str.c_str());
  return s;
}
char* copy(String str,char* def) {  
  if(str==NULL || str==EMPTYSTRING) { return def; } 
  int len  =str.length()+1; if(len==0) { return def; } char ca[len]; str.toCharArray(ca,len); return(ca);
}

//------------------------------------------------------------------

/* extract from src (NEW char[]) (e.g. is=extract(".",":","This.is:new") )*/
char* extract(char *start, char *end, char *src) {
    const char *start_ptr=src;
    if(is(start)) {  // find start if given
      start_ptr = strstr(src, start); 
      if (!start_ptr) { return NULL; } 
      else { start_ptr += strlen(start); }  // Move past 'start'
    }      
    size_t len = 0;
    if(is(end)) { 
      const char *end_ptr = strstr(start_ptr, end); if (!end_ptr) { return NULL; }
      len = end_ptr - start_ptr; 
    }else  {
      len=strlen(start_ptr);
    }
    char *result=newChar(len+1);
    strncpy(result, start_ptr, len);  result[len] = '\0';  
    return result;
}

