# CPTP
remote data sync protocol developed by C<br>
推荐使用:<br> [C应用程序框架/C APP FRAME](https://github.com/bedreamer/capp.git)<br>
Demo: <br>[CPTP DOG DEVELOPED BY Python](https://github.com/bedreamer/ptpdog.git)<br>
<pre>
protocol point define:
    member no.1: id
    member no.2: value

protocol usage:
  usage 1:
  ========
      sigle master node and multiple slave node.
      +---------+          +---------+            +---------+
      +  <b>master</b> +          + slave 1 +      ...   + slave n +
      +----.----+          +----.----+            +----.----+
           |                    |                      | 
     ==================================================================== BUS(RS485)
     timing:
     t0
        master send reuqest frame to slave 1
     t1
        slave send ack frame to master
     ....
     tn-1
        master send request frame to slave n/2
     tn slave n/2 send ack frame to master
  usage 2:
  ========
      multiple node mode
      +---------+          +---------+            +---------+
      +  node 1 +          +  node 2 +      ...   +  node n +
      +----.----+          +----.----+            +----.----+
           |                    |                      | 
     ==================================================================== BUS(CAN, ETH)
     node 1,2,..n could send request frame to BUS anytime. and order by rule as follow:
     T(request) node A send request to node B
     T(ack)  node B send ack to node A
  usage 3:
  =========
      global brodcast or group brodcast
      +---------+          +---------+            +---------+            +---------+
      +  <b>master</b> +          + slave 1 +            + slave n +      ...   + slave n +
      +         +          +   <b>G1</b>    +            +   <b>G1</b>    +      ...   +   Gn    +
      +----.----+          +----.----+            +----.----+            +----.----+
           |                    |                      | 
     ==================================================================== BUS(RS485)
     timing:
     T0:
         master send a global broadcast frame to G1.
     T1:
         G1 member do some action but no any ack frame send to BUS.
  
</pre>
