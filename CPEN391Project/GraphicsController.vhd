LIBRARY ieee; 
USE ieee.Std_Logic_1164.all;
use ieee.Std_Logic_arith.all; 
use ieee.Std_Logic_signed.all;  
--use ieee.numeric_std.all;

   
entity GraphicsController is  
	Port (  
 
-- Signals from NIOS Bridge (i.e. CPU) interface signals
		AddressIn 										: in  Std_Logic_Vector(15 downto 0);			-- CPU address bus
		DataInFromCPU 									: in  Std_Logic_Vector(15 downto 0);			-- 16 bit data bus from CPU
 
  		Clk,
		Reset_L 											: in Std_Logic;		
		
		AS_L												: in Std_Logic; 										-- address strobe indicates address is valid
		UDS_L												: in Std_Logic; 										-- upper and lower data strobes indicates which halves of the data bus are being used by NIOS
		LDS_L												: in Std_Logic;  
		RW 												: in Std_Logic;										-- RW CPU Read = 1, CPU Write = 0
		
-- VGA display driver signals (the circuit actually displaying the contents of the frame buffer)
		
		GraphicsCS_L									: in Std_Logic; 		 		 						-- Chip select signal from NIOS Bridge shows graphics is being accessed by NIOS
		VSync_L											: in Std_Logic;										-- Horizontal and Vertical sync from VGA display controller
		SRam_DataIn										: in Std_Logic_Vector(15 downto 0);				-- 16 bit data bus in from Memory holding the image to be displayed
		
-- Scroll offset signal (currently disabled and thus outputting zero on both)
		
		VScrollValue 									: out Std_Logic_Vector(9 downto 0);  			-- scroll value for terminal emulation (allows screen scrolling up/down) currently not used
		HScrollValue 									: out Std_Logic_Vector(9 downto 0);  			-- scroll value for terminal emulation (allows screen scrolling left/right) currently not used

-- Data bus back to CPU		
		DataOutToCPU									: out Std_Logic_Vector(15 downto 0);			-- 16 bit data bus back to Bridge/NIOS when reading from graphis

-- Memory output signals
-- Memory holds the graphics image that we see and draw - we have to dri these signals to create the image as part of a state machine
-- Memory is 16 wide, split into 2 x 8 bit halves
-- each location is thus 2 bytes and holds two pixels from our image
		
		Sram_AddressOut								: out Std_Logic_Vector(17 downto 0);			-- graphics controller address out to memory 256k locations = 18 address lines
		Sram_DataOut									: out Std_Logic_Vector(15 downto 0);			-- graphics controller Data out (data gets written to memory via this)
		
		Sram_UDS_Out_L									: out Std_Logic; 	-- Upper Data Strobe : Drive to '0' if graphics wants to read/write from/to lower byte in memory
		Sram_LDS_Out_L									: out Std_Logic;  -- Lower Data Strobe : Drive to '0' if graphics wants to read/write from/to upper byte in memory
		Sram_RW_Out										: out Std_Logic;	-- Read/Write : Drive to '0' if graphics wants to write or '1' if graphics wants to read
		
		ColourPalletteAddr							: out Std_Logic_Vector(7 downto 0) ;			-- an address connected to programmable colour pallette (256 colours)
		ColourPalletteData							: out Std_Logic_Vector(31 downto 0) ;			-- 24 bit 00RRGGBB value to write to the colour pallette
		ColourPallette_WE_H							: out Std_Logic										-- signal to actually write to colour pallette (data and address must be valid at this time)
	);
end;
 
architecture bhvr of GraphicsController is
-- 16 bit registers that can be written to by NIOS each is mapped to an address in memory
-- X1,Y1 and X2,Y2 can be used to represent coords, e.g. draw a pixel or draw a line from x1,y1 to x2,y2
-- CPU writes values to these registers and the graphcis controller will do the rest

	Signal 	X1, Y1, X2, Y2, Colour, BackGroundColour, Command 		: Std_Logic_Vector(15 downto 0);	

-- 16 bit register that can be read by NIOS. It holds the 8 bit pallette number of the pixel that we read (see reading pixels)
	
	Signal 	Colour_Latch														: Std_Logic_Vector(15 downto 0);			-- registers
	
	-- signals to control the registers above
	Signal 	X1_Select_H, 
   			X2_Select_H,
				Y1_Select_H, 
				Y2_Select_H,
				Command_Select_H,
				Colour_Select_H,
				BackGroundColour_Select_H: Std_Logic; 	
	
	Signal 	CommandWritten_H, ClearCommandWritten_H,							-- signals to control that a command has bee written to the graphcis by NIOS
				Idle_H, SetBusy_H, ClearBusy_H	: Std_Logic;					-- signals to control status of the graphics chip				
	
	-- Temporary Asynchronous signals that drive the Ram (made synchronous in a register for the state machine)
	-- your VHDL code should drive these signals not the real Sram signals.
	-- These signals are copied to the real Sram signals with each clock
	
	Signal 	Sig_AddressOut 																	: Std_Logic_Vector(17 downto 0);
	Signal 	Sig_DataOut 																		: Std_Logic_Vector(15 downto 0); 
	Signal 	Sig_UDS_Out_L, 
				Sig_LDS_Out_L, 
				Sig_RW_Out,Sig_UDS_Out_temp_L, Sig_LDS_Out_temp_L,Sig_RW_Out_temp : Std_Logic;
	Signal	Sig_Busy_H																			: Std_Logic;
	
	Signal	Colour_Latch_Load_H																: Std_Logic;
	Signal	Colour_Latch_Data																	: Std_Logic_Vector(15 downto 0);
		
	-- Colour Pallette signals
	Signal	Sig_ColourPalletteAddr															: Std_Logic_Vector(7 downto 0) ;
	Signal	Sig_ColourPalletteData															: Std_Logic_Vector(31 downto 0) ;
	Signal	Sig_ColourPallette_WE_H															: Std_Logic; 			

	-- Signal to tell the graphics controller when it is safe to read/write to the graphcis memory
	-- this is when the display controller is not actually accessing the memory to display it's contents to the screen
	-- i.e. during a horizontal sync period
	-- you should only attempt to read/write memory when this signal is '0'
	
	 
-------------------------------------------------------------------------------------------------------------------------------------------------
-- States and signals for state machine
-------------------------------------------------------------------------------------------------------------------------------------------------
	
	-- signals that hold the current state and next state of your state machine at the heart of our graphcis controller
	Signal 	CurrentState, 
				NextState 			: Std_Logic_Vector(7 downto 0);
	 
	-- here are some state numbers associated with some functionality already present in the graphics controller, e.g.
	-- writing a pixel, reading a pixel, programming a colour pallette
	--
	-- you will be adding new states so make sure you have unique values for each state (no duplicate values)
	-- e.g. DrawHLine does not do anything yet - you have add the code to that state to draw a line
	
	constant Idle						 				: Std_Logic_Vector(7 downto 0) := X"00";		-- main waiting state
	constant ProcessCommand			 				: Std_Logic_Vector(7 downto 0) := X"01";		-- State is figure out command
	constant DrawHline			 	 				: Std_Logic_Vector(7 downto 0) := X"02";		-- State for drawing a Horizontal line
	constant DrawVline			 	 				: Std_Logic_Vector(7 downto 0) := X"03";		-- State for drawing a Vertical line
	constant DrawLine				 	 				: Std_Logic_Vector(7 downto 0) := X"04";		-- State for drawing any line
	constant DrawPixel							 	: Std_Logic_Vector(7 downto 0) := X"05";		-- State for drawing a pixel
	constant ReadPixel							 	: Std_Logic_Vector(7 downto 0) := X"06";		-- State for reading a pixel
	constant ReadPixel1							 	: Std_Logic_Vector(7 downto 0) := X"07";		-- State for reading a pixel
	constant ReadPixel2							 	: Std_Logic_Vector(7 downto 0) := X"08";		-- State for reading a pixel
	constant PalletteReProgram						: Std_Logic_Vector(7 downto 0) := X"09";		-- State for programming a pallette
	
	--clarence added for exercise1.7
	-----draw H line----
	-----Assumption-----
	-----x, y, length, color------
	constant DrawHline1   							: Std_Logic_Vector(7 downto 0) := X"10";		-- State for updating x coord
	constant DrawHline2  							: Std_Logic_Vector(7 downto 0) := X"11";		-- State for drawing a pixel in X reg
	
	constant DrawVline1   							: Std_Logic_Vector(7 downto 0) := X"12";		-- State for updating x coord
	constant DrawVline2  							: Std_Logic_Vector(7 downto 0) := X"13";		-- State for drawing a pixel in X reg
	
	constant Drawline1   							: Std_Logic_Vector(7 downto 0) := X"14";		-- State for updating x coord
	constant Drawline2  								: Std_Logic_Vector(7 downto 0) := X"15";		-- State for drawing a pixel in X reg
	constant Drawline3  								: Std_Logic_Vector(7 downto 0) := X"16";
	constant Drawline4  								: Std_Logic_Vector(7 downto 0) := X"17";
	
	Signal	X	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	X_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	X_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	Signal	Y	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	Y_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	Y_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	
	---------------------------for draw line algorithm--------------------------------------------
	Signal	interchange	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	interchange_Data  	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	interchange_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	Signal	x2Minusx1	: std_logic_vector(15 downto 0) ;
	Signal	y2Minusy1	: std_logic_vector(15 downto 0) ;
	
	Signal	S1	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	S1_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	S1_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	Signal	S2	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	S2_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	S2_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	Signal	SwapX	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	SwapX_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'

	Signal	SwapY	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	SwapY_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'	
	
	Signal	Swap_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	Signal	Error	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	Error_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	Error_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	Signal	For_counter	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	For_counter_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	For_counter_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	
	Signal	dx	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	dx_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	dx_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	Signal	dy	: std_logic_vector(15 downto 0) ;	-- the variable/register 'x‘ 16 bits wide
	Signal	dy_Data 	: std_Logic_Vector(15 downto 0);	-- signal/wire carrying data to be stored in 'x'
	Signal	dy_Load_H 	: std_logic;			-- signal/wire to store/update ‘x’
	
	
	-- add any extra states you need here for example to draw lines etc.
-------------------------------------------------------------------------------------------------------------------------------------------------
-- Commands that can be written to command register by NIOS to get graphics controller to draw a shape
-------------------------------------------------------------------------------------------------------------------------------------------------
	constant Hline								 		: Std_Logic_Vector(15 downto 0) := X"0001";	-- command to Graphics chip from NIOS is draw Horizontal line
	constant Vline									 	: Std_Logic_Vector(15 downto 0) := X"0002";	-- command to Graphics chip from NIOS is draw Vertical line
	constant ALine									 	: Std_Logic_Vector(15 downto 0) := X"0003";	-- command to Graphics chip from NIOS is draw any line
	constant	PutPixel									: Std_Logic_Vector(15 downto 0) := X"000a";	-- command to Graphics chip from NIOS to draw a pixel
	constant	GetPixel									: Std_Logic_Vector(15 downto 0) := X"000b";	-- command to Graphics chip from NIOS to read a pixel
	constant ProgramPallette						: Std_Logic_Vector(15 downto 0) := X"0010";	-- command to Graphics chip from NIOS is program one of the pallettes with a new RGB value
Begin



---------------------------------------------------------clarence----------------------------------------------------------------------------------
--command that load X_data to X------------------------------------------------------ 
process(Clk)
begin
	if(rising_edge(Clk)) then
		if(X_Load_H = '1') then	-- if load active (generated by your state machine)
			X <= X_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(Y_Load_H = '1') then	-- if load active (generated by your state machine)
			Y <= Y_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

-------------------------------draw line algorithm
process(Clk)
begin
	if(rising_edge(Clk)) then
		if(interchange_Load_H = '1') then	-- if load active (generated by your state machine)
			interchange <= interchange_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(S1_Load_H = '1') then	-- if load active (generated by your state machine)
			S1 <= S1_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(S2_Load_H = '1') then	-- if load active (generated by your state machine)
			S2 <= S2_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(Swap_Load_H = '1') then	-- if load active (generated by your state machine)
			SwapX <= SwapX_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(Swap_Load_H = '1') then	-- if load active (generated by your state machine)
			SwapY <= SwapY_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(Error_Load_H = '1') then	-- if load active (generated by your state machine)
			Error <= Error_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(For_counter_Load_H = '1') then	-- if load active (generated by your state machine)
			For_counter <= For_counter_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(dx_Load_H = '1') then	-- if load active (generated by your state machine)
			dx <= dx_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

process(Clk)
begin
	if(rising_edge(Clk)) then
		if(dy_Load_H = '1') then	-- if load active (generated by your state machine)
			dy <= dy_Data;	-- copy data on X_Data to X on next rising edge
		end if;
	end if;
end process;

------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Secondary CPU address decoder within chip                                  
-- This logic decodes the address coming out of the Bridge (via NIOS) and decides which internal graphics registers NIOS is accessing
-- 
------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	process(AddressIn, GraphicsCS_L, RW, AS_L, Idle_H)
	begin
		-- these are important "default" values for the signals. Remember back in VHDL lectures how we supply default values for signals
		-- to stop the VHDL compiler inferring latches
		-- these defaults are the "inactive" values for those signals. They get overridden below.
		
		X1_Select_H 					<= '0';
		Y1_Select_H 					<= '0';
		X2_Select_H 					<= '0';
		Y2_Select_H 					<= '0';
		Colour_Select_H 				<= '0';
		BackGroundColour_Select_H 	<= '0';
		Command_Select_H 				<= '0';
	
		-- if 16 bit Bridge address outputs addres in range hex 0000 - 00FF then Graphics chip will be be accessed
		-- remember the bridge itself is activtated when NIOS reads/write to location hex 0400_xxxx
		-- you should read/write to locations hex 8400_xxxx to bypass any data cache in NIOS II/f CPU
		
		if(GraphicsCS_L = '0' and RW = '0' and AS_L = '0' and (AddressIn(15 downto 8) = X"00")) then		
			if 	(AddressIn(7 downto 1) = B"0000_000")	then		Command_Select_H <= '1';							-- Command reg is at address hex 8400_0000
			elsif	(AddressIn(7 downto 1) = B"0000_001") 	then		X1_Select_H <= '1';									-- X1 reg is at address hex 8400_0002
			elsif	(AddressIn(7 downto 1) = B"0000_010")	then		Y1_Select_H <= '1';									-- Y1 reg is at address hex 8400_0004
			elsif	(AddressIn(7 downto 1) = B"0000_011")	then		X2_Select_H <= '1';									-- X2 reg is at address hex 8400_0006
			elsif	(AddressIn(7 downto 1) = B"0000_100")	then		Y2_Select_H <= '1';									-- Y2 reg is at address hex 8400_0008
			elsif	(AddressIn(7 downto 1) = B"0000_111")	then		Colour_Select_H <= '1';								-- Colour reg is at address hex 8400_000E
			elsif (AddressIn(7 downto 1) = B"0001_000") 	then		BackGroundColour_Select_H <= '1';				-- Background colour reg reg is at address hex 8400_0010
			end if;
		end if;
	end process;
	
 
------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- CommandWritten Process
-- This process sets CommandWritten_H to '1' when NIOS writes to Graphics Command register
-- in other words it kick starts the graphics controller into doing something when NIOS gives a command to the graphics controller
------------------------------------------------------------------------------------------------------------------------------
	process(Clk, Reset_L)
	Begin
		if(Reset_L = '0') then								-- clear all registers and relevant signals on reset (asynchronous to clock)
			CommandWritten_H <= '1';						-- SIMULATION ONLY set back to 0 after
		elsif(rising_edge(Clk)) then
			if(Command_Select_H = '1') then				-- when CPU writes to command register
				CommandWritten_H <= '1';
			elsif(ClearCommandWritten_H = '1') then	-- signal to clear the register after graphics controller has dealt with the command
				CommandWritten_H <= '0';
			end if;
		end if;
	end process;

	
------------------------------------------------------------------------------------------------------------------------------
-- Process to allow NIOS to Read Graphics Chip Status
-- it's activated when CPU reads status reg of graphics chip
-- the Following data is supplied back to NIOS as "status"
------------------------------------------------------------------------------------------------------------------------------
	process(GraphicsCS_L, AddressIn, RW, AS_L, Idle_H, Colour_Latch)
	Begin
		DataOutToCPU <= "ZZZZZZZZZZZZZZZZ";																		-- default is tri state data-out bus to CPU
		
		if(GraphicsCS_L = '0' and RW = '1' and AS_L = '0') then 
			if(AddressIn(15 downto 1) = B"0000_0000_0000_000") then										-- read of status register at address 8400_0000
				DataOutToCPU <= B"000000000000000"  & Idle_H;												-- leading 15bits of 0 plus idle status on bit 0
			elsif(AddressIn(15 downto 1) = B"0000_0000_0000_111") then									-- read of colour register hex 0e/0f
				DataOutToCPU <= Colour_Latch ;
			end if ;
		end if;
	end process;

------------------------------------------------------------------------------------------------------------------------------
-- Busy_Idle Process
-- This updates the status of the Graphics chip so it can be read by NIOS
------------------------------------------------------------------------------------------------------------------------------
	process(Clk, Reset_L)
	Begin
		if(Reset_L = '0') then								-- clear all registers and relevant signals on reset (asynchronous to clock)
			Idle_H <= '1';										-- make sure CPU can read graphics as idle
		elsif(rising_edge(Clk)) then						-- when graphics starts to process a command, mark it's status as busy (not idle) when CPU writes to command register		
			if(SetBusy_H = '1') then						
				Idle_H <= '0';
			elsif(ClearBusy_H = '1') then					-- when done processing command update status
				Idle_H <= '1';
			end if;
		end if;
	end process;	

------------------------------------------------------------------------------------------------------------------------------
-- X1 Process
-- This process stores the 16 value from NIOS into the X1 register
--
-- You could add extra functionality to this process if you like, e.g. increment x1 when a signal from state machine says so
-- this might be useful when drawing a horizontal line for example, where you increment x1 until it equals x2
------------------------------------------------------------------------------------------------------------------------------
	process(Clk, Reset_L)
	Begin
		if(Reset_L = '0') then
			X1 <= X"0000" ;
		elsif(rising_edge(Clk)) then
			if(X1_Select_H = '1') then	
				if(UDS_L = '0') then
					X1(15 downto 8) <= DataInFromCPU(15 downto 8);
				end if ;				
				if(LDS_L = '0') then
					X1(7 downto 0) <= DataInFromCPU(7 downto 0);
				end if ;
			end if;
		end if;
	end process;
	
------------------------------------------------------------------------------------------------------------------------------
-- Y1 Process
-- This process stores the 16 value from NIOS into the Y1 register
--
-- You could add extra functionality to this process if you like, e.g. increment y1 when a signal from state machine says so
-- this might be useful when drawing a vertical line for example, where you increment y1 until it equals y2
------------------------------------------------------------------------------------------------------------------------------
	process(Clk, Reset_L)
	Begin	
		if(Reset_L = '0') then
			Y1 <= X"0000" ;
		elsif(rising_edge(Clk)) then
			if(Y1_Select_H = '1') then
				if(UDS_L = '0') then
					Y1(15 downto 8) <= DataInFromCPU(15 downto 8);
				end if;
				if(LDS_L = '0') then
					Y1(7 downto 0) <= DataInFromCPU(7 downto 0);
				end if;
			end if;
		end if;
	end process;
	
------------------------------------------------------------------------------------------------------------------------------
-- X2 Process
------------------------------------------------------------------------------------------------------------------------------
	process(Clk, Reset_L)
	Begin
		if(Reset_L = '0') then
			X2 <= X"0000" ;
		elsif(rising_edge(Clk)) then
			if(X2_Select_H = '1') then				
				if(UDS_L = '0') then
					X2(15 downto 8) <= DataInFromCPU(15 downto 8);
				end if;
				if(LDS_L = '0') then
					X2(7 downto 0) <= DataInFromCPU(7 downto 0);
				end if;
			end if;
		end if;
	end process;		
	
------------------------------------------------------------------------------------------------------------------------------
-- Y2 Process
------------------------------------------------------------------------------------------------------------------------------
	process(Clk, Reset_L)
	Begin
		if(Reset_L = '0') then
			Y2 <= X"0000" ;
		elsif(rising_edge(Clk)) then
			if(Y2_Select_H = '1') then				
				if(UDS_L = '0') then
					Y2(15 downto 8) <= DataInFromCPU(15 downto 8);
				end if;
				if(LDS_L = '0') then
					Y2(7 downto 0) <= DataInFromCPU(7 downto 0);
				end if;
			end if;
		end if;
	end process;	
	
------------------------------------------------------------------------------------------------------------------------------
-- Colour Reg Process
--
-- This process stores the colour of the "thing" e.g. pixel, line etc that we are going to draw
------------------------------------------------------------------------------------------------------------------------------
	process(Clk, Reset_L)
	Begin
		if(Reset_L = '0') then
			Colour <= X"0004" ;					-- Colour Pallette number 4 (blue) so screen is erased to blue on reset
		elsif(rising_edge(Clk)) then
			if(Colour_Select_H = '1') then				
				if(UDS_L = '0') then
					Colour(15 downto 8) <= DataInFromCPU(15 downto 8);
				end if;
				if(LDS_L = '0') then
					Colour(7 downto 0) <= DataInFromCPU(7 downto 0);
				end if;
			end if;
		end if;
	end process;	
	
------------------------------------------------------------------------------------------------------------------------------
-- Background Colour Reg Process
--
-- This process stores the background colour of the "thing" we are drawing
-- at the moment it is used only when drawing characters. Characters have displayable pixels (that govern the look of the character)
-- and non-displayable pixels that should be the background colour, e.g. the Letter 'O' has a circle of displayable pixels
-- and a big hole of non displayable pixels in the middle. When we display 'O' we want to set the circle pixels to the "colour" 
-- and the hole pixels to the backgroun colour (you decide what the background should be bu you write it's pallatte number in the background
-- colour register
------------------------------------------------------------------------------------------------------------------------------
	process(Clk)
	Begin
		if(rising_edge(Clk)) then
			if(BackGroundColour_Select_H = '1') then				
				if(UDS_L = '0') then
					BackGroundColour(15 downto 8) <= DataInFromCPU(15 downto 8);
				end if;
				if(LDS_L = '0') then
					BackGroundColour(7 downto 0) <= DataInFromCPU(7 downto 0);
				end if;
			end if;
		end if;
	end process;		
		
------------------------------------------------------------------------------------------------------------------------------
-- Command Process
--
-- IMPORTANT - NIOS writes to this register to ask the graphics controller to "draw something"
-- you must ensure you have set up any relevent registers such as x1, y1 and colour (for drawing a pixel for example)
-- or x1,y1,x2,y2 registers for drawing a line etc
--
-- As soon as NIOS writes to the command register the graphics chip will start to draw the shape
-- make sure you check the status of the graphics chip to make sure it is idle before writing to ANY of it's registers
------------------------------------------------------------------------------------------------------------------------------
	process(Clk, Reset_L)
	Begin
		if(Reset_L = '0') then		-- clear all registers and relevant signals on reset (asynchronous to clock)
			Command <= X"0000";		
		elsif(rising_edge(Clk)) then
			if(Command_Select_H = '1') then				
				if(UDS_L = '0') then
					Command(15 downto 8) <= DataInFromCPU(15 downto 8);
				end if;
				if(LDS_L = '0') then
					Command(7 downto 0) <= DataInFromCPU(7 downto 0);
				end if;
			end if;
		end if; 
	end process;	

------------------------------------------------------------------------------------------------------------------------------
-- Colour Latch Process (used for reading pixel)
--
-- This process holds the data read from memory when we read a pixel
------------------------------------------------------------------------------------------------------------------------------
	process(Clk)
	Begin
		if(rising_edge(Clk)) then
			if(Colour_Latch_Load_H = '1') then				
				Colour_Latch <= Colour_Latch_Data;
			end if ;
		end if;
	end process;		
	
------------------------------------------------------------------------------------------------------------------------------------------------------------------------
--	State Machine Registers and XY clipper prevents write if off the screen
------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Process(Clk, Reset_L, Sig_AddressOut, Colour_latch_load_H, Sig_UDS_Out_temp_L, Sig_LDS_Out_temp_L, Sig_RW_Out_temp)
	Begin
	
		if(Reset_L = '0') then
			CurrentState <= Idle; 					-- on reset enter the idle state (always a good idea!!)
			
		elsif(Rising_edge(Clk)) then
			CurrentState <= NextState;				-- with each clock move to the next state (which could be same state if there is nothing to do)

-----------------------------------------------------------------------------------------------------------------------------------------------------			
-- IMPORTANT -  Make outputs to the Sram SYNCHRONOUS - here, on each rising edge of the clock we copy all our calculated values to the actual Sram memory
--
-- It's important to remember that there is a 1 clock cycle delay from when we generate our calculated signals to when they are
-- actually presented to the graphics controller - remember the "MOORE" model of a state machine
-----------------------------------------------------------------------------------------------------------------------------------------------------			
			
			Sram_AddressOut		<= Sig_AddressOut ;			-- On rising edge of clock copy our calculated address to the Sram
			Sram_DataOut 			<= Sig_DataOut;				-- On rising edge of clock copy our calculated data to the Sram
			Sram_UDS_Out_L 		<= Sig_UDS_Out_L;				-- On rising edge of clock copy our calculated UDS value to the Sram
			Sram_LDS_Out_L 		<= Sig_LDS_Out_L;				-- On rising edge of clock copy our calculated LDS value to the Sram
			Sram_RW_Out				<= Sig_RW_Out;					-- On rising edge of clock copy our calculated R/W value to the Sram
			
			-- Graphics scroll signal updates
			VScrollValue 			<= B"00_0000_0000"; 							-- no scrolling implemented so output 0
			HScrollValue			<= B"00_0000_0000"; 							-- no scrolling implemented so output 0
			
			-- copy colour pallette calculated signals to colour pallette memory
			ColourPalletteAddr	<= Sig_ColourPalletteAddr;
			ColourPalletteData	<= Sig_ColourPalletteData;
			ColourPallette_WE_H	<= Sig_ColourPallette_WE_H; 
	
		end if; 
	end process;
	
---------------------------------------------------------------------------------------------------------------------
-- next state and output logic
----------------------------------------------------------------------------------------------------------------------	
	
	process(CurrentState, CommandWritten_H, Command, X1, X2, Y1, Y2, Colour, VSync_L,
				BackGroundColour, AS_L, Sram_DataIn, CLK, Colour_Latch)
	begin
	
	----------------------------------------------------------------------------------------------------------------------------------
	-- IMPORTANT
	-- start with default values for EVERY signal (so we do not infer storage for signals inside this process_
	-- and override as necessary.
	-- 
	-- If you ad any new signals to the logic, you MUST supply a default value - it's VHDL remember
	-----------------------------------------------------------------------------------------------------------------------------------
		Sig_AddressOut 					<= B"00_0000_0000_0000_0000";			-- got to supply something so it might as well be address 0
		Sig_DataOut 						<= Colour(7 downto 0) & Colour(7 downto 0);		-- default is to output the value of the colour registers to the Sram data bus
		Sig_UDS_Out_L 						<= '1';	-- assume upper data bus NOT being accessed
		Sig_LDS_Out_L 						<= '1';	-- assume lower data bus NOT being accessed
		Sig_RW_Out 							<= '1';	-- assume reading
		
		ClearBusy_H 						<= '0';	-- default is do NOT Clear busy
		SetBusy_H							<= '0';	-- default is do NOT Set busy
		ClearCommandWritten_H			<= '0';	-- default is no command has been written by nios
		Sig_Busy_H							<= '1';	-- default is device is busy
	
		Colour_Latch_Load_H				<= '0';					-- default is NOT to load colour latch
		Colour_Latch_Data					<= X"0000";				-- default data to colour latch is 0
			
		Sig_ColourPalletteAddr			<= X"00";				-- default address to the colour pallette
		Sig_ColourPalletteData			<= X"00000000" ;		-- default 00RRGGBB value to the colour pallette
		Sig_ColourPallette_WE_H			<= '0'; 					-- default is NO write to the colour pallette
		
		-------------------------------------------------------------------------------------
		-- IMPORTANT we have to define what the default NEXT state will be. In this case we the state machine
		-- will return to the IDLE state unless we override this with a different one
		-------------------------------------------------------------------------------------
		
		NextState							<= Idle;	
				
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------				
		if(CurrentState = Idle ) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			ClearBusy_H <= '1';							-- mark status as idle
			Sig_Busy_H <= '0';							-- show graphics outside world that it is NOT busy
			
			-- if NIOS is writing to command register
			if(CommandWritten_H = '1') then
				if(AS_L = '0') then						-- if NIOS still writing the command, stay in idle state
					NextState <= IDLE;
				else											-- once NIOS has finished start processing the command by moving to a new state
					NextState <= ProcessCommand;
				end if;
			end if;
			
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------			
		elsif(CurrentState = ProcessCommand) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			SetBusy_H <= '1';								-- set the busy status of the graphics chip
			ClearCommandWritten_H <= '1';				-- clear the command that NIOS wrote, now that we are processing it
			
			-- decide what command NIOS wrote and move to a new state to deal with that command
			
			if(Command = PutPixel) then
				NextState <= DrawPixel;	
			elsif(Command = GetPixel) then
				NextState <= ReadPixel;	
			elsif(Command = ProgramPallette) then
				NextState <= PalletteReProgram ;				
			elsif(Command = Hline) then
				NextState <= DrawHLine;
			elsif(Command = Vline) then
				NextState <= DrawVline;
			elsif(Command = ALine) then
				NextState <= DrawLine;	
				
			-- add other code to process any new commands here e.g. draw a circle if you decide to implement that
			-- or draw a rectangle etc
			
			end if;

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		elsif(CurrentState = PalletteReProgram) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

-- This state is responsible for programming 1 of the 256 colour pallettes with a new 00RRGGBB value
-- Your program/NIOS should have written the pallette number to the "Colour" register 
-- and the 32 bit 00RRGGBB value to the pallette into X1 (mist significant 16 bits i.e. 00RR) 
-- and Y1 (least significant 16 bits i.e. GGBB) before writing to the command register with a "program pallette" command)

			Sig_ColourPalletteAddr 	<= Colour(7 downto 0);		-- The pallette number to program should be in Colour Register. 
			Sig_ColourPalletteData	<= X1 & Y1;						-- 00RRGGBB		
			
			-- to avoid flicker on screen, we only reprogram a pallette during the vertical sync period, 
			-- i.e. at the end of the last frame and before the next one
			
			if(VSync_L = '0') then										-- if VGA is not displaying at this time, then we can program pallette, otherwise wait for video blanking during Vsync
				Sig_ColourPallette_WE_H	 <= '1';						-- issue the actual write signal to the colour pallette during a Vertical sync
				NextState <= IDLE;
			else
				NextState <= PalletteReProgram;						-- stay here until pallette has been reprogrammed
			end if ;
			
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		elsif(CurrentState = DrawPixel) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- This state is responsible for drawing a pixel to an X,Y coordinate on the screen
-- Your program/NIOS should have written the colour pallette number to the "Colour" register 
-- and the coords to the x1 and y1 register before writing to the command register with a "Draw pixel" command)

			
-- the address of the pixel is formed from the 9 bit y coord that indicates a "row" (1 out of a maximum of 512 rows)
-- coupled with a 9 bit x or column address within that row. Note a 9 bit X address is used for a maximum of 1024 columns or horizontal pixels
-- You might thing that 10 bits would be required for 1024 columns and you would be correct, except that the address we are issuing
-- holds two pixels (the memory us 16 bit wide remember so each location/address is that of 2 pixels)
				
			Sig_AddressOut 	<= Y1(8 downto 0) & X1(9 downto 1);				-- 9 bit x address even though it goes up to 1024 which would mean 10 bits, because each address = 2 pixels/bytes
			Sig_RW_Out			<= '0';													-- we are intending to draw a pixel so set RW to '0' for a write to memory
			
			if(X1(0) = '0')	then														-- if the address/pixel is an even numbered one
				Sig_UDS_Out_L 	<= '0';													-- enable write to upper half of Sram data bus to access 1 pixel at that location
			else
				Sig_LDS_Out_L 	<= '0';													-- else write to lower half of Sram data bus to get the other pixel at that address
			end if;
				
			-- the data that we write comes from the default value assigned to Sig_DataOut previously
			-- you will recall that this is the value of the Colour register
				
			NextState <= IDLE;		-- return to idle state after writing pixel

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		elsif(CurrentState = ReadPixel) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			-- present the X/Y address to the memory chip as we did when writing a pixel (see above)
			-- it will take 2 clock to get the data back
			-- 1 clock will be required for the state machine here to output these signal to the sram (on next risgin edge when we change to state ReadPixel1)
			-- plus 1 more clock for the synchronous ram to latch the address and respond with the data (when we change to the state ReadPixel2)
			
			Sig_AddressOut 	<= Y1(8 downto 0) & X1(9 downto 1);				-- 8 bit x address even though it goes up to 1024 which would mean 10 bits, because each address = 2 pixles/bytes
			
			-- we are going to read both pixels from the memory chup at this address/location
			Sig_UDS_Out_L 	<= '0';														-- enable read from  upper half of Sram data bus
			Sig_LDS_Out_L 	<= '0';
			
			NextState <= ReadPixel1;
			
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		elsif(CurrentState = ReadPixel1) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			-- when we reach this state, graphics controller will have output address etc to the sram (but the sram will not have clocked it in yet)
			-- (it's synchronous ram remember) so on the next clock (next state), it will respond to the address here
			NextState <= ReadPixel2 ;		-- dummy state to allow signals to reach video ram frame buffer
				
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		elsif(CurrentState = ReadPixel2) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			-- when we reach this state, sram fram buffer will have clocked in the signals above and after a few nS will be outputting the data
			-- for us to grab
			
			Colour_Latch_Load_H 				<= '1';										-- set up signal to grabstore colour data

			if(X1(0) = '0')	then															-- if the address/pixel is an even numbered one
				Colour_Latch_Data(7 downto 0) <= SRam_DataIn(15 downto 8);		-- grab upper byte/pixel value
			else
				Colour_Latch_Data(7 downto 0) <= SRam_DataIn(7 downto 0);		-- otherwise grab lower byte/pixel instead
			end if;				
			
			NextState <= IDLE ;		--  move to a terminating read state
			
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		elsif(CurrentState = DrawHline) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			-- TODO in your project
			
			----clarence------
			----store X1 into X
			X_data <= X1; 			
			X_Load_H <= '1';    ----let X load data from X_data
			
			---go to draw a pixel
			NextState <= DrawHline1;
		
		elsif(CurrentState = DrawHline1)then
		
			Sig_AddressOut 	<= Y1(8 downto 0) & X(9 downto 1);				-- 9 bit x address even though it goes up to 1024 which would mean 10 bits, because each address = 2 pixels/bytes
			Sig_RW_Out			<= '0';													-- we are intending to draw a pixel so set RW to '0' for a write to memory
			----Y is not changing, we use X for x coordinate
			
			if(X(0) = '0')	then														-- if the address/pixel is an even numbered one
				Sig_UDS_Out_L 	<= '0';													-- enable write to upper half of Sram data bus to access 1 pixel at that location
			else
				Sig_LDS_Out_L 	<= '0';													-- else write to lower half of Sram data bus to get the other pixel at that address
			end if;
		
			NextState <= DrawHline2;
			
		elsif(CurrentState = DrawHline2)then
			
			if(X < X2) then
				X_Data <= X + 1;
				NextState <= DrawHline1;
			else
				X_load_H <= '0';
				NextState <= IDLE;
			end if;
				
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		elsif(CurrentState = DrawVline) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------		
			-- TODO in your project
			----clarence------
			----store Y1 into Y
			Y_data <= Y1 ; ---not sure if 1 is the right amount to increment			
			Y_Load_H <= '1';    ----let Y load data from Y_data
			
			---go to draw a pixel
			NextState <= DrawVline1;
		
		elsif(CurrentState = DrawVline1)then
		
			Sig_AddressOut 	<= Y(8 downto 0) & X1(9 downto 1);				-- 9 bit x address even though it goes up to 1024 which would mean 10 bits, because each address = 2 pixels/bytes
			Sig_RW_Out			<= '0';													-- we are intending to draw a pixel so set RW to '0' for a write to memory
			----X is not changing, we use Y for x coordinate
			
			if(X1(0) = '0')	then														-- if the address/pixel is an even numbered one
				Sig_UDS_Out_L 	<= '0';													-- enable write to upper half of Sram data bus to access 1 pixel at that location
			else
				Sig_LDS_Out_L 	<= '0';													-- else write to lower half of Sram data bus to get the other pixel at that address
			end if;
		
			NextState <= DrawVline2;
			
		elsif(CurrentState = DrawVline2)then
			
			if(Y < Y2) then
				Y_data <= Y + 1;
				NextState <= DrawVline1;
			else
				Y_load_H <= '0';
				NextState <= IDLE;
			end if;
			
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		elsif(CurrentState = DrawLine) then
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------		
			-- TODO in your project----
			------------clarence-----
			
			
			
			
			X_Data <= X1;	-- copy x1 register to the temporary ‘x’ register – i.e. int x = x1;
			Y_Data <= Y1;	-- copy y1 register to the temporary ‘y’ register – i.e. int y = y1;

			X_Load_H 	<= '1';
			Y_Load_H 	<= '1';
			
			x2Minusx1 := X2 - X1; 	 -- calculations 
			y2Minusy1 := Y2 - Y1;	 -- calculations
			
			dx_Data <= abs(signed(x2Minusx1));		--  int dx = abs(x2 - x1);
			dy_Data <= abs(signed(y2Minusy1));		--  int dy = abs(y2 - y1);

			dx_Load_H 	<= '1';
			dy_Load_H 	<= '1';

			interchange_Data 	<= X"0000";		-- equivalent of interchange = 0
			
			interchange_Load_H <= '1';	

			-- calculate equivalent of C expression s1= sign(x2 - x1)
			if(x2Minusx1 < 0) then
				S1_Data <= X"FFFF";			-- s1 = -1 in 2’s complement
			elsif (x2Minusx1 = 0) then
				S1_Data <= X"0000";			-- s1 = 0
			else
				S1_Data <= X"0001";			-- s1 = 1
			end if;
			
			S1_Load_H 	<= '1';

			-- calculate equivalent of C expression s2= sign(y2 - y1)
			if(y2Minusy1 < 0) then	
				S2_Data <= X"FFFF";	-- s2 = -1 in 2’s complement
			elsif (y2Minusy1 = 0) then
				S2_Data <= X"0000";	-- s2 = 0
			else
				S2_Data <= X"0001";	-- s2 = 1
			end if;	
				
			S2_Load_H 	<= '1';
			
			------------clarence---------------
			----changed the provided so that if dx or dy ==0, return
			if(dx_Data = 0 and dy_Data = 0) then
				NextState <= IDLE;
			else 
				NextState <= DrawLine1;
			end if;
			
			----store dx into swap for swapping later-----
			SwapX_Data <= dx_Data;
			SwapY_Data <= dy_Data;
			
			----if dy > dx, swap
			if(dy_Data>dx_Data) then
				Swap_Load_H <= '1';
				interchange_Data = X"0001";
			else
				Swap_Load_H <= '0';
			end if;
			
		elsif(CurrentState = DrawLine1) then
		------do the swap-----------dx, dy
			if(Swap_Load_H = '1') then
				dy_Data <= SwapX;    --dy = dx
				dx_Data <= SwapY;	 --dx = dy
			end if;
			
			--there might be timing error here
			Error_Data <= (dy(14 downto 0) & '0') - dx_Data ; --Error = (dy<<1) -dx
			Error_Load_H <= '1';
			
			For_counter_Data <= X"0001";
			For_counter_Load_H <= '1';
			
			nextState <= DrawLine2;
		
		elsif(CurrentState = DrawLine2) then
			--------step into main loop------------
			--------if i<= dx---------------
			if(For_counter <= dx) then
				------------increment counter-------------
				For_counter_Data <= For_counter + 1;
				
				----------write a pixel---------------
				Sig_AddressOut 	<= Y(8 downto 0) & X(9 downto 1);				-- 9 bit x address even though it goes up to 1024 which would mean 10 bits, because each address = 2 pixels/bytes
				Sig_RW_Out			<= '0';													-- we are intending to draw a pixel so set RW to '0' for a write to memory
				----Y is not changing, we use X for x coordinate
			
				if(X(0) = '0')	then														-- if the address/pixel is an even numbered one
					Sig_UDS_Out_L 	<= '0';													-- enable write to upper half of Sram data bus to access 1 pixel at that location
				else
					Sig_LDS_Out_L 	<= '0';													-- else write to lower half of Sram data bus to get the other pixel at that address
				end if;
					
				nextState <= DrawLine3;
			else
				nextState <= IDLE; --i>dx, finish
			end if;
			
		elsif(CurrentState = DrawLine3) then
		------------------correct error---------------
			if(Error >= 0) then
				if(interchange = 1) then
					X_Data <= X + S1;
				else
					Y_Data <= Y + S2;
				end if;
				
				Error_Data <= Error - (dx(14 downto 0) & '0');  --Error = Error - dx<<1
				nextState <= DrawLine3;
				
			else
				nextState <= DrawLine4;
			end if ;
			
		elsif(CurrentState = DrawLine4) then
		--------------------interchange branch--------------
			if(interchange = 1) then
				Y_Data <= Y + S2;
			else
				X_Data <= X + S1;
			end if;
			
			Error_Data <= Error + (dy(14 downto 0) & '0');
			Error_Data <= Error + (dy(14 downto 0) & '0'); --Error = Error + dy<<1
			
			nextState <= DrawLine2;
			
			
----------------------------------------------------------------------------------------------------------------
		end if ;
	end process;	
end;
