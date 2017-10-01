----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 25.07.2017 20:04:53
-- Design Name: 
-- Module Name: interruptTimer - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity interruptTimer is
    generic (
        DATA_WIDTH : integer := 32
    );
    Port ( clk : in STD_LOGIC;
           interrupt_A : in STD_LOGIC;
           interrupt_B : in STD_LOGIC;
           interrupt_out : out STD_LOGIC;
           data : out STD_LOGIC_VECTOR (DATA_WIDTH-1 downto 0));
end interruptTimer;

architecture Behavioral of interruptTimer is

type control is (IDLE, FIRST_INTERRUPT, CAPTURED);

signal counter_int : unsigned(DATA_WIDTH-1 downto 0) := (others => '0');

signal state : control := IDLE;

begin

process(clk)
begin 
    if rising_edge(clk) then
    
        if state = IDLE and interrupt_A = '1' then 
            state <= FIRST_INTERRUPT;
            counter_int <= (others => '0');
        elsif state = FIRST_INTERRUPT and interrupt_B = '1' then
            state <= CAPTURED;
            interrupt_out <= '1';
        elsif state = FIRST_INTERRUPT then
            counter_int <= counter_int + 1;
        elsif state = CAPTURED then
            interrupt_out <= '0';
            state <= IDLE;
        end if;

    end if;
end process;

data <= std_logic_vector(counter_int);

end Behavioral;
