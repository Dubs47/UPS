package sample;

import javafx.application.Platform;
import javafx.scene.control.Button;
import javafx.stage.Stage;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.SocketTimeoutException;

/**
 * Represents thread, which receives messages from server and process them
 */
public class ReceivingHandle implements Runnable
{
    private static final int MATRIX_SIZE = 25;
    private volatile boolean exit = false;
    private Socket socket;
    private BufferedReader input;
    private PrintWriter output;
    private Stage primaryStage;
    private String message;
    private Button[][] matrix = new Button[MATRIX_SIZE][MATRIX_SIZE];
    PingHandle ph;
    private int timeout;

    /**
     *
     * @param input             input stream
     * @param output            output stream
     * @param primaryStage      stage
     * @param socket            socket
     * @param ph                ping thread
     */
    public ReceivingHandle(BufferedReader input, PrintWriter output, Stage primaryStage, Socket socket, PingHandle ph)
    {
        this.input = input;
        this.output = output;
        this.primaryStage = primaryStage;
        this.socket = socket;
        this.message = null;
        this.ph = ph;
        this.timeout = 0;
    }

    public void run()
    {
        matrixInit();

        do {
            try {
                while ((message = input.readLine()) != null)
                {
                    if(timeout > 0)
                    {
                        output.println("ping\0");
                        timeout = 0;
                        Platform.runLater(() -> Windows.popupWindow("Information", "Reconnection successful!"));
                        Main.setReconnecting(false);
                    }
                    processMessage(message);
                    if (exit)
                        return;
                }
            } catch (SocketTimeoutException e)
            {
                Main.setReconnecting(true);
                timeout++;
                if(timeout < 3)
                {
                    Platform.runLater(() -> Windows.popupWindow("Warning", "Connection lost, reconnecting..."));
                }
                output.println("ping\0");
            }
            catch (IOException e) {
                break;
            }
        } while (timeout < 3);

        Platform.runLater(() -> Windows.popupWindow("Error", "Connection lost!!!"));
        Platform.runLater(() -> Main.connectWindow(primaryStage));
        ph.stop();
        stop();
        try {
            input.close();
            input = null;
            output.close();
            output = null;
            socket.close();
            socket = null;
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Creates matrix with no symbols
     */
    public void matrixInit()
    {
        for (int i = 0; i < matrix.length; i++)
        {
            for (int j = 0; j < matrix[0].length; j++)
            {
                Button b = new Button(" ");
                matrix[i][j] = b;
            }
        }
    }

    /**
     * Ends thread
     */
    public void stop()
    {
        exit = true;
    }

    /**
     * Process message
     *
     * @param message       message from server
     */
    public void processMessage(String message)
    {
        String[] token = message.split(";");
        if(token.length == 1)
        {
            if(message.equals("server exit"))
            {
                Platform.runLater(() -> Windows.popupWindow("Error", "Server went down"));
                Main.setId(null);
                Main.setNickname(null);
                Platform.runLater(() -> Main.connectWindow(primaryStage));
            }
            if(message.equals("Exit or play?"))
            {
                Platform.runLater(() -> Windows.lobbyWindow(primaryStage, output, this, ph, false));
            }
            else if(message.equals("reconnected"))
            {
                Main.setReconnecting(false);
                Platform.runLater(() -> Windows.popupWindow("Information", "Reconnection successful!!!"));
            }
            else if(message.equals("session expired"))
            {
                Main.setReconnecting(false);
                Platform.runLater(() -> Windows.popupWindow("Warning", "You lost connection for more than 1 minute, new session created"));
            }
            else if(message.equals("ping"))
            {
                return;
            }
            else if(message.equals("Waiting for second player"))
            {
                Platform.runLater(() -> Windows.popupWindow("Information", "Waiting for second player"));
                Platform.runLater(() -> Windows.lobbyWindow(primaryStage, output, this, ph, true));
            }
            else if(message.equals("Your turn"))
            {
                Platform.runLater(() -> Windows.popupWindow("Information", "Your turn"));
                Platform.runLater(() -> Windows.gameWindow(primaryStage, output, matrix, "me"));
            }
            else if(message.equals("Enemy turn"))
            {
                Platform.runLater(() -> Windows.popupWindow("Information", "Enemy turn"));
                Platform.runLater(() -> Windows.gameWindow(primaryStage, output, matrix, "enemy"));
            }
            else if(message.equals("disconnected enemy"))
            {
                Platform.runLater(() -> Windows.popupWindow("Warning", "Enemy disconnected, returning to lobby"));
                output.println("Enter lobby\0");
            }
            else if(message.equals("win"))
            {
                Platform.runLater(() -> Windows.popupWindow("Information", "YOU HAVE WON!!!!"));
                output.println("Enter lobby\0");
            }
            else if(message.equals("lose"))
            {
                Platform.runLater(() -> Windows.popupWindow("Information", "YOU HAVE LOST!!!!"));
                output.println("Enter lobby\0");
            }
            else
            {
                output.println("Bad message, disconnecting...");
                Platform.runLater(() -> Windows.popupWindow("Error", "Bad message from server, disconnecting"));
                Platform.runLater(() -> Main.connectWindow(primaryStage));
                try {
                    socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                stop();
                ph.stop();
            }
        }
        else if(token.length == 3)
        {
            String symbol = token[0];
            if (symbol.equals("X") || symbol.equals("O"))
            {
                int x = -1, y = -1;
                try {
                    x = Integer.parseInt(token[1]);
                    y = Integer.parseInt(token[2]);
                }
                catch (NumberFormatException e)
                {
                    output.println("Bad massage, disconnecting...");
                    Platform.runLater(() -> Windows.popupWindow("Error", "Bad massage from server, disconnecting..."));
                    Platform.runLater(() -> Main.connectWindow(primaryStage));
                    try {
                        socket.close();
                    }
                    catch (IOException ioe)
                    {

                    }
                    stop();
                }
                if (x < 0 || x >= MATRIX_SIZE || y < 0 || y >= MATRIX_SIZE)
                {
                    output.println("Bad massage, disconnecting...");
                    Platform.runLater(() -> Windows.popupWindow("Error", "Bad massage from server, disconnecting..."));
                    Platform.runLater(() -> Main.connectWindow(primaryStage));
                    try {
                        socket.close();
                    }
                    catch (IOException ioe)
                    {

                    }
                    stop();
                    return;
                }
                matrix[x][y].setText(symbol);
                matrix[x][y].setDisable(true);
            }
            else if(token[0].equals("login"))
            {
                if(token[1] == null || token[2] == null)
                {
                    output.println("Bad massage, disconnecting...");
                    Platform.runLater(() -> Windows.popupWindow("Error", "Bad massage from server, disconnecting..."));
                    Platform.runLater(() -> Main.connectWindow(primaryStage));
                    try {
                        socket.close();
                    }
                    catch (IOException e)
                    {

                    }
                    ph.stop();
                    stop();
                }
                else
                {
                    Main.setId(token[1]);
                    Main.setNickname(token[2]);
                    Platform.runLater(() -> Windows.popupWindow("Information", "Hello " + Main.getNickname()));
                }
            }
            else if(token[0].equals("Player found!"))
            {
                if(token[1] == null || token[2] == null || token[1].length() != 1)
                {
                    output.println("Bad massage, disconnecting...");
                    Platform.runLater(() -> Windows.popupWindow("Error", "Bad massage from server, disconnecting..."));
                    Platform.runLater(() -> Main.connectWindow(primaryStage));
                    try {
                        socket.close();
                    }
                    catch (IOException e)
                    {

                    }
                    ph.stop();
                    stop();
                }
                else
                {
                    matrixInit();
                    Main.setMySymbol(token[1]);
                    Main.setEnemyNickname(token[2]);
                }
            }
            else
            {
                output.println("Bad massage, disconnecting...");
                Platform.runLater(() -> Windows.popupWindow("Error", "Bad massage from server, disconnecting..."));
                Platform.runLater(() -> Main.connectWindow(primaryStage));
                try {
                    socket.close();
                }
                catch (IOException e)
                {

                }
                stop();
            }

        }

        // Bad format
        else
        {
            output.println("Bad massage, disconnecting...");
            Platform.runLater(() -> Windows.popupWindow("Error", "Bad massage from server, disconnecting..."));
            Platform.runLater(() -> Main.connectWindow(primaryStage));
            try {
                socket.close();
            }
            catch (IOException e)
            {

            }
            stop();
        }
    }
}
