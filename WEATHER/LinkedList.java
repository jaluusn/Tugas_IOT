//belajar algoritma



class Node {
    String  nama;
    Node next; 
}

public class LinkedList {
    
    public static void main(String[]  Args){
         
         Node current = null;
         
         Node first   = new Node();
         Node seccond = new Node();
         Node third   = new Node();
         
         
         
         first.nama  = "Hawi";
         first.next  = seccond;
         
         seccond.nama   = "gopal";
         third.next    = third;
         
         third.nama   = "kdlw";
         third.next   =  null;
         
         current     = first;
         
         while (current !=null){
             System.out.println(current.nama);
             current = current.next;
         };
         
    }
    
}