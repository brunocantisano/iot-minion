// Tipos.h
#ifndef TIPOS_H_INCLUDED
#define TIPOS_H_INCLUDED
// ou: #pragma once

class Application {
public:
  String name;
  String language;
  String description;
};

class Media {
public:
  String name;
  String path;
  int size;
  String lastModified;
};

class ArduinoSensorPort {
public:
  int id;
  int gpio;
  String name;
  bool status;
};

#endif // TIPOS_H_INCLUDED
