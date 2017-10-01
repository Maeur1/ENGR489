----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 17.07.2017 07:46:57
-- Design Name: 
-- Module Name: rgmii_detector_top - Behavioral
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
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity rgmii_detector_top is
    Port ( input_rxc : in STD_LOGIC;
           input_rx_ctl : in STD_LOGIC;
           recieved : out STD_LOGIC);
end rgmii_detector_top;

architecture Behavioral of rgmii_detector_top is

component edge_detector is
    Port ( clk : in STD_LOGIC;
           ddr_line : in STD_LOGIC;
           active : out STD_LOGIC);
end component;

begin

detect_in: edge_detector port map (
    clk => input_rxc,
    ddr_line => input_rx_ctl,
    active => recieved
);


end Behavioral;
