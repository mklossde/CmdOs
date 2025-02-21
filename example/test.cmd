$a = 1

# if ##############################################

if $a == 1 goto #one
error "wrong if-goto"
#one
if $a == 2 {}
else goto #two
error "wrong else-goto"
#two
if $a == 2 {}
elseif $a == 1 goto #three
error "wrong elseif-goto"
#three

if $a == 1 {
 $a = $a + 1
}

if $a == 1 {
  error "if-wrong"
}else if $a == 2 {
  $a = $a + 1
}

if $a == 1 {
	error "if-wrong"
}elseif $a == 2 {
	error "elseif-wrong" 
} else {
	$a = $a + 1
}

if $a == 4 { 
  if $a == 5 error "inner if wrong"
  if $a == 5 {
    error "inner if wrong"
  }elseif $a == 5 {
    error "inner if2 wrong"
  } elseif $a == 4 {
	if $a == 4 {}	  
	else {
	  error "inner2 else wrong"
	}
  } else {
     error "inner else wrong"
  }
}

# until ##############################################

$a = 1
{
  $a = $a + 1
} until $a > 10

log "TEST DONE OK"

