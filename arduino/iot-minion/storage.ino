// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println(F("Listando arquivos armazenados no storage"));
  File root = LittleFS.open("/", "r");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Nome</th><th align='left'>Tamanho</th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      int tam = strlen(foundfile.name());
      char temp[tam];
      strncpy(temp,foundfile.name(),tam);
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td></tr>";
    } else {
      returnText += "Arquivo: " + String(foundfile.name()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}
