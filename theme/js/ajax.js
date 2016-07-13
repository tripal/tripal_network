function loadDoc(str)
 {
  var xhttp;
  
  if(window.XMLHttpRequest)
  {
    xhttp = new XMLHttpRequest();
  }
  else
  {
    xhttp = new ActiveXObject("Microsoft.XMLHTTP");
  }

  xhttp.onreadystatechange=function()
  {
    if(xhttp.readyState==4 && xhttp.status==200)
    {
      document.getElementById("modules").innerHTML=xhttp.responseText;
      


    }
  };
  xhttp.open("POST","retrieve.php?q="+str,true);
  xhttp.send();
 }