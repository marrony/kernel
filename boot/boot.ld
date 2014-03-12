SECTIONS
{
    .text 0x7c00 : {
        *(.text);
    }

     /*. = 0x7c00;*/

     /*.text :
     {
        *(.text);
     }*/

     /*.text :
     {
          _ftext = .;
     } = 0*/
}
