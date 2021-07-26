import java.io.File;
import java.io.FileWriter;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Scanner;
import com.fazecast.jSerialComm.SerialPort;

/**
 * class for testing out the USB serial library jSerialComm
 * An Arduino Uno is used as a dummy to receive and answer commands
 * (Arduino Program: Java_Program_Tester.ino)
 * @author Philipp Schulz
 */
public class PortalRadioTester
{
    // port of the MCU over USB
    static SerialPort temporaryPort;
    static SerialPort COMPort;
    static File PortFile;

    /**
     * main method and therefore starting point of the program
     * @author Philipp Schulz
     */
    public static void main(String[] args)
    {
        USBLibraryUsage();
    }

    /**
     * method for trying out the USB serial library
     * @author Philipp Schulz
     */
    private static void USBLibraryUsage()
    {
        // get a list of all available COM ports
        SerialPort[] portNames = SerialPort.getCommPorts();
        // initialize index of working port
        int COMPortIndex = -1;
        // initialize Streams
        InputStream USBInputStream;
        OutputStream USBOutputStream;
        Scanner USBInputScanner;

        // check if file with found COM port is already present
        boolean fileFound = false;
        String storedCOMPortName = "";
        try
        {
            PortFile = new File("COMPort.txt");
            Scanner fileReader = new Scanner(PortFile);
            storedCOMPortName = fileReader.next();

            fileReader.close();
            // loop over ports from SerialPort
            for (SerialPort portName : portNames)
            {
                if (portName.getSystemPortName().equals(storedCOMPortName))
                {
                    fileFound = true;
                    break;
                }
            }

        } catch(Exception e)
        {
            System.out.println("File not found. Searching for COM port instead.");
        }

        // if the file was not found
        if(!fileFound)
        {
            // loop over all found ports
            for(int i = 0; i < portNames.length; i++)
            {
                // check if COM port is available and uses the defined protocol
                try
                {
                    // get selected port
                    temporaryPort = SerialPort.getCommPort(portNames[i].getSystemPortName());
                    // set timeouts
                    temporaryPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 2000, 2000);

                    // check if connection was successful
                    if(temporaryPort.openPort())
                    {
                        System.out.println("Testing "+ portNames[i].getSystemPortName());
                        // wait for micro controller to restart
                        Thread.sleep(4000);
                        // create IO streams
                        USBInputStream = temporaryPort.getInputStream();
                        USBOutputStream = temporaryPort.getOutputStream();
                        USBInputScanner = new Scanner(USBInputStream);
                        // send INIT command
                        USBOutputStream.write("INIT/#/".getBytes());
                        // receive answer
                        String answer1 = USBInputScanner.nextLine();
                        String answer2 = USBInputScanner.nextLine();
                        // check if answer is right
                        if(answer2.contains("OK")&&answer2.contains("/#/"))
                        {
                            // set current index as the COMPortIndex
                            COMPortIndex = i;
                            System.out.println("Communication successful!");
                            // close port
                            temporaryPort.closePort();

                            // write found port to file
                            PortFile = new File("COMPort.txt");
                            if(!PortFile.exists())
                            {
                                PortFile.createNewFile();
                            }
                            FileWriter PortFileWriter = new FileWriter("COMPort.txt");
                            PortFileWriter.write(portNames[i].getSystemPortName()+" "+answer1);
                            PortFileWriter.close();

                            // break the loop
                            break;
                        }
                    }
                } catch(Exception e)
                {
                    System.out.println("Communication failed.");
                    // close  port
                    temporaryPort.closePort();
                }
            }
        } else
        {
            // loop over found ports
            for(int i = 0; i < portNames.length; i++)
            {
                // check if port from file is in this list
                if(portNames[i].getSystemPortName().equals(storedCOMPortName))
                {
                    // set the correct index
                    COMPortIndex = i;
                    // exit the loop to save time
                    break;
                }
            }
        }

        // if a suitable COM port was found
        if(COMPortIndex >= 0)
        {
            System.out.println("Suitable device found at "+portNames[COMPortIndex].getSystemPortName()+". Starting communication test");
            try
            {
                // get selected port
                COMPort = SerialPort.getCommPort(portNames[COMPortIndex].getSystemPortName());
                // set timeouts
                COMPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING,0 ,0);
                // check if connection is successful
                if(COMPort.openPort())
                {// wait for micro controller to restart
                    Thread.sleep(4000);
                    // get required streams
                    USBInputStream = COMPort.getInputStream();
                    USBOutputStream = COMPort.getOutputStream();
                    USBInputScanner = new Scanner(USBInputStream);

                    // Scanner and String for command line inputs
                    Scanner terminalInputScanner = new Scanner(System.in);
                    String terminalInput="";

                    // get device name
                    USBOutputStream.write("INIT/#/".getBytes());
                    // receive all answers
                    ArrayList<String> answersTemp = new ArrayList<>();
                    String receivedDataTemp = "";
                    do
                    {
                        receivedDataTemp = USBInputScanner.nextLine();
                        answersTemp.add(receivedDataTemp);
                    } while(!receivedDataTemp.contains("/#/"));
                    String deviceName = answersTemp.get(0);

                    System.out.println("Starting transmission tests");
                    // initialize received data container
                    String receivedData;
                    System.out.println("The following commands are available:");
                    System.out.println("INIT");
                    System.out.println("STATUS");
                    System.out.println("RADIO ON|OFF");
                    System.out.println("VOLUME HOME|[numeric volume in percent]");
                    System.out.println("SPEAKER RESET");
                    System.out.println("exit (to exit this program)");
                    do
                    {
                        System.out.print("Input: ");
                        terminalInput = terminalInputScanner.nextLine();
                        if(!terminalInput.contains("exit"))
                        {
                            // construct and send data to microcontroller
                            String command = terminalInput+"/#/";
                            USBOutputStream.write(command.getBytes());
                            // receive all answers
                            ArrayList<String> answers = new ArrayList<>();
                            do
                            {
                                receivedData = USBInputScanner.nextLine();
                                answers.add(receivedData);
                            } while(!receivedData.contains("/#/"));

                            if(terminalInput.contains("INIT"))
                            {
                                deviceName = answers.get(0);
                            }
                            System.out.println(deviceName+": " + answers);
                        }
                    }while(!terminalInput.contains("exit"));



                    System.out.println("Tests finished. Exiting program");
                    // close port
                    COMPort.closePort();
                }
            } catch(Exception e)
            {
                System.out.println("An error has occurred:");
                e.printStackTrace();
            }
        }
    }
}
