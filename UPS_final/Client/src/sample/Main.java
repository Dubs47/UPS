package sample;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.GridPane;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class Main extends Application {
    static Socket socket = null;
    static PrintWriter output = null;
    static BufferedReader input = null;
    static PingHandle ph = null;
    static ReceivingHandle rh = null;
    private static String id = null, nickname = null, enemyNickname = null, mySymbol;
    private static boolean reconnecting = false;

    public static String getEnemyNickname() {
        return enemyNickname;
    }

    public static void setEnemyNickname(String enemyNickname) {
        Main.enemyNickname = enemyNickname;
    }

    public static String getMySymbol() {
        return mySymbol;
    }

    public static void setMySymbol(String mySymbol) {
        Main.mySymbol = mySymbol;
    }

    public static String getId()
    {
        return id;
    }

    public static void setId(String newId)
    {
        id = newId;
    }

    public static String getNickname()
    {
        return nickname;
    }

    public static void setNickname(String newNickname)
    {
        nickname = newNickname;
    }

    public static boolean getReconnecting()
    {
        return reconnecting;
    }

    public static void setReconnecting(boolean newReconnecting)
    {
        reconnecting = newReconnecting;
    }

    /**
     * Stop all threads on exit
     */
    /*
    @Override
    public void stop(){
        if(socket != null) {
            try {
                output.println(":exit\0");
                socket.close();
                if(ph != null)
                    ph.stop();
                if (rh != null)
                    rh.stop();
            } catch (IOException e) {

            }
        }
    }

     */

    public void stop()
    {
        System.exit(0);
    }

    @Override
    public void start(Stage primaryStage) throws Exception {
        //Parent root = FXMLLoader.load(getClass().getResource("sample.fxml"));
        primaryStage.setOnCloseRequest(new EventHandler<WindowEvent>() {
            @Override
            public void handle(WindowEvent event) {
                if(socket != null) {
                    try {
                        output.println(":exit\0");
                        socket.close();
                        if(ph != null)
                            ph.stop();
                        if (rh != null)
                            rh.stop();
                    } catch (IOException e) {

                    }
                }
                Platform.exit();
            }
        });
        connectWindow(primaryStage);
    }

    /**
     * Represents window to connect to server
     *
     * @param primaryStage      stage
     */
    public static void connectWindow(Stage primaryStage)
    {
        GridPane pane = new GridPane();
        TextField address = new TextField();
        TextField port = new TextField();
        Label labelAddress = new Label("Address:");
        Label labelPort = new Label("Port");
        Button button = new Button("Submit");
        button.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                tryConnect(address.getText(), port.getText(), primaryStage);
            }
        });

        pane.add(labelAddress, 1, 1);
        pane.add(address, 1, 2);
        pane.add(labelPort, 1, 4);
        pane.add(port, 1, 5);
        pane.add(button, 4, 6);

        primaryStage.setTitle("Client");
        primaryStage.setScene(new Scene(pane, 800, 700));
        primaryStage.show();
    }

    /**
     * Obtains connection to server
     *
     * @param address           address of server
     * @param p                 port of server
     * @param primaryStage      stage
     */
    public static void tryConnect(String address, String p, Stage primaryStage)
    {
        if(address.equals("") || p.equals(""))
        {
            Windows.popupWindow("Warning", "Address or port missing");
            return;
        }

        int port = -1;
        while (port < 0)
        {
            try {
                port = Integer.parseInt(p);
            }
            catch (NumberFormatException e)
            {
                Windows.popupWindow("Warning", "Port is number");
                return;
            }
        }

        try {
            socket = new Socket(address, port);
            socket.setSoTimeout(10000);
            output = new PrintWriter(socket.getOutputStream(), true);
            input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            ph = new PingHandle(output);
            rh = new ReceivingHandle(input, output, primaryStage, socket, ph);
            Thread th = new Thread(rh);
            th.start();
            Thread th2 = new Thread(ph);
            th2.start();
            Windows.popupWindow("Information", "Connected to server\n");

            // Player is reconnecting
            if(reconnecting && nickname != null && id != null)
            {
                output.println("reconnect;" + id + ";" + nickname);
            }

            // Player connecting first time
            else
            {
                Windows.nicknameWindow(primaryStage, output);
            }
        } catch (IOException u)
        {
            Windows.popupWindow("Error", "Cannot reach server");
        }
    }

    /**
     * entry point
     *
     * @param args      input arguments
     */
    public static void main(String[] args)
    {
        launch(args);
    }
}
