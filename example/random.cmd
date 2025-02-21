log "find random number"
$a = random 1 99
$g = 50
$t = 1

#loop
  log "your guess" $g "==" $a "try:"$t

  $t = $t + 1
  if $t > 10 {
    log "to many trys" $t
    end
  }
  if $g > $a {
    log "to high"
    $g = $g / 2
    goto #loop
  }
  if $g < $a {
    log "to low"
    $x = $g / 2
    $g = $g + $x
    goto #loop
  }

  if $g != $a goto #loop
  

#end 
  log "FOUND" $a
