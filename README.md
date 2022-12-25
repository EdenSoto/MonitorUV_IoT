# MonitorUV_IoT
Monitor UV IoT  con servicios AWS

Es un proyecto realizada con la finalidad de monitorear los rayos UV que pueden causar daños a las personas por una expocición prolongada.
El proyecto usa el sensor ML8511 UV y DHT11, el primero nos entraga una señal analogica que seran convertidoes en niveles de radiación y 
el sendo sensor nos entrada señales digitales de temperratura y humedad relativa.
El prototipo se conecta a los servicios de Amazon primero a AWS IoT core por el protocolo MQTT y esta se vincula a una maquina vitual que 
se encuentra en otro sercicio el cual es Aws EC2 el cual cumple la funcion de servidor en la que esta instalada Rede-node y esta muestra 
mediante un dashboard toda la información de los sensores.

Para más detalle pude ingresar a: https://www.notion.so/Proyecto-Monitor-UV-con-AWS-8c1650e03280411f8ef8edf9c60404e2

![diseñoFinal](https://user-images.githubusercontent.com/105142766/209456939-083d94e0-af6e-48c6-be04-3ed832dbf653.png)

![Imagen de WhatsApp 2022-12-10 a las 11 18 38](https://user-images.githubusercontent.com/105142766/209457081-1f2d1c9b-4a50-42b1-9960-0d5372935dc3.jpg)

![captura 05-12-2022(211823)EDENSS](https://user-images.githubusercontent.com/105142766/209456968-40344148-bddb-4606-bc4b-c11dcefdffe8.png)
