static void retro_defaultkeyboard()
{
//	log_message(keyboard_log, "loading default retro keymap start:");

    if ( keyconvmap != NULL )
	{
        keyboard_keyconvmap_free();
    }

    keyboard_keyconvmap_alloc();
	keyboard_keyword_clear();

#ifdef __PLUS4__

	joystick_joypad_clear();
	kbd_lshiftrow = 1;
	kbd_lshiftcol = 7;
	joystick_joypad_clear();
	kbd_rshiftrow = 1;
	kbd_rshiftcol = 7;
 	joystick_joypad_clear();
	vshift=0;
	joystick_joypad_clear();
	shiftl=KEY_LSHIFT;
	joystick_joypad_clear();

	keyboard_parse_set_pos_row(  8, 0, 0, 8);               /*    Backspace -> Inst/Del     */
//	keyboard_parse_set_pos_row(277, 0, 0, 8);               /*       Insert -> Inst/Del     */
	keyboard_parse_set_pos_row( 13, 0, 1, 8);               /*       Return -> Return       */
	keyboard_parse_set_pos_row(125, 0, 2, 8);               /*            ] -> &            */
	keyboard_parse_set_pos_row(285, 0, 3, 8);               /*           F4 -> Help         */
	keyboard_parse_set_pos_row(285, 0, 3, 1);               /*    shift+ F4 -> F7           */
	keyboard_parse_set_pos_row(282, 0, 4, 8);               /*           F1 -> F1           */
	keyboard_parse_set_pos_row(282, 0, 4, 1);               /*    shift+ F1 -> F4           */
	keyboard_parse_set_pos_row(283, 0, 5, 8);               /*           F2 -> F2           */
	keyboard_parse_set_pos_row(283, 0, 5, 1);               /*    shift+ F2 -> F5           */
	keyboard_parse_set_pos_row(284, 0, 6, 8);               /*           F3 -> F3           */
	keyboard_parse_set_pos_row(284, 0, 6, 1);               /*    shift+ F3 -> F6           */
	keyboard_parse_set_pos_row(123, 0, 7, 8);               /*            [ -> @            */
	
	keyboard_parse_set_pos_row( 51, 1, 0, 8);               /*            3 -> 3            */
	keyboard_parse_set_pos_row(119, 1, 1, 8);               /*            W -> W            */
	keyboard_parse_set_pos_row( 97, 1, 2, 8);               /*            A -> A            */
	keyboard_parse_set_pos_row( 52, 1, 3, 8);               /*            4 -> 4            */
	keyboard_parse_set_pos_row(122, 1, 4, 8);               /*            Z -> Z            */
	keyboard_parse_set_pos_row(115, 1, 5, 8);               /*            S -> S            */
	keyboard_parse_set_pos_row(101, 1, 6, 8);               /*            E -> E            */
	keyboard_parse_set_pos_row(304, 1, 7, 2);               /*   Left Shift -> SHIFT        */
	keyboard_parse_set_pos_row(303, 1, 7, 2);               /*  Right Shift -> SHIFT        */
	keyboard_parse_set_pos_row(301, 1, 7, 64);              /*    Caps Lock -> SHIFT LOCK   */
	
	keyboard_parse_set_pos_row( 53, 2, 0, 8);               /*            5 -> 5            */
	keyboard_parse_set_pos_row(114, 2, 1, 8);               /*            R -> R            */
	keyboard_parse_set_pos_row(100, 2, 2, 8);               /*            D -> D            */
	keyboard_parse_set_pos_row( 54, 2, 3, 8);               /*            6 -> 6            */
	keyboard_parse_set_pos_row( 99, 2, 4, 8);               /*            C -> C            */
	keyboard_parse_set_pos_row(102, 2, 5, 8);               /*            F -> F            */
	keyboard_parse_set_pos_row(116, 2, 6, 8);               /*            T -> T            */
	keyboard_parse_set_pos_row(120, 2, 7, 8);               /*            X -> X            */
	
	keyboard_parse_set_pos_row( 55, 3, 0, 8);               /*            7 -> 7            */
	keyboard_parse_set_pos_row(121, 3, 1, 8);               /*            Y -> Y            */
	keyboard_parse_set_pos_row(103, 3, 2, 8);               /*            G -> G            */
	keyboard_parse_set_pos_row( 56, 3, 3, 8);               /*            8 -> 8            */
	keyboard_parse_set_pos_row( 98, 3, 4, 8);               /*            B -> B            */
	keyboard_parse_set_pos_row(104, 3, 5, 8);               /*            H -> H            */
	keyboard_parse_set_pos_row(117, 3, 6, 8);               /*            U -> U            */
	keyboard_parse_set_pos_row(118, 3, 7, 8);               /*            V -> V            */
	
	keyboard_parse_set_pos_row( 57, 4, 0, 8);               /*            9 -> 9            */
	keyboard_parse_set_pos_row(105, 4, 1, 8);               /*            I -> I            */
	keyboard_parse_set_pos_row(106, 4, 2, 8);               /*            J -> J            */
	keyboard_parse_set_pos_row( 48, 4, 3, 8);               /*            0 -> 0            */
	keyboard_parse_set_pos_row(109, 4, 4, 8);               /*            M -> M            */
	keyboard_parse_set_pos_row(107, 4, 5, 8);               /*            K -> K            */
	keyboard_parse_set_pos_row(111, 4, 6, 8);               /*            O -> O            */
	keyboard_parse_set_pos_row(110, 4, 7, 8);               /*            N -> N            */
	
	keyboard_parse_set_pos_row(274, 5, 0, 8);               /*         Down -> CRSR DOWN    */
	keyboard_parse_set_pos_row(112, 5, 1, 8);               /*            P -> P            */
	keyboard_parse_set_pos_row(108, 5, 2, 8);               /*            L -> L            */
	keyboard_parse_set_pos_row(273, 5, 3, 8);               /*           Up -> CRSR UP      */
	keyboard_parse_set_pos_row( 46, 5, 4, 8);               /*            . -> .            */
	keyboard_parse_set_pos_row( 59, 5, 5, 8);               /*            ; -> :            */
	keyboard_parse_set_pos_row( 45, 5, 6, 8);               /*        Minus -> MINUS        */
	keyboard_parse_set_pos_row(279, 5, 6, 8);               /*        End   -> MINUS        */
	keyboard_parse_set_pos_row( 44, 5, 7, 8);               /*            , -> ,            */

	keyboard_parse_set_pos_row(276, 6, 0, 8);               /*         Left -> CRSR LEFT    */
	keyboard_parse_set_pos_row(126, 6, 1, 8);               /*        Tilde -> *            */
	keyboard_parse_set_pos_row( 92, 6, 1, 8);               /*  Bkslash(US) -> *            */
	keyboard_parse_set_pos_row( 39, 6, 2, 8);               /*            ' -> ;            */
	keyboard_parse_set_pos_row(275, 6, 3, 8);               /*        Right -> CRSR RIGHT   */
	keyboard_parse_set_pos_row( 27, 6, 4, 8);               /*          ESC -> ESC          */
	keyboard_parse_set_pos_row( 61, 6, 5, 8);               /*       Equals -> EQUALS       */
	keyboard_parse_set_pos_row(281, 6, 5, 8);               /*    Page Down -> EQUALS       */
	keyboard_parse_set_pos_row(127, 6, 6, 8);               /*          Del -> +            */
	keyboard_parse_set_pos_row( 47, 6, 7, 8);               /*            / -> /            */
	
	keyboard_parse_set_pos_row( 49, 7, 0, 8);               /*            1 -> 1            */
	keyboard_parse_set_pos_row(278, 7, 1, 8);               /*         Home -> CLR/HOME     */
	keyboard_parse_set_pos_row(  9, 7, 2, 8);               /*          TAB -> Ctrl         */
	keyboard_parse_set_pos_row( 50, 7, 3, 8);               /*            2 -> 2            */
	keyboard_parse_set_pos_row( 32, 7, 4, 8);               /*        Space -> Space        */
	keyboard_parse_set_pos_row(306, 7, 5, 8);               /*    Left Ctrl -> CBM          */ 
	keyboard_parse_set_pos_row(113, 7, 6, 8);               /*            Q -> Q            */
	keyboard_parse_set_pos_row( 19, 7, 7, 8);               /*        Break -> Run/Stop     */

#else //

#error Unsupported key mapping.

#endif // platform.

}

