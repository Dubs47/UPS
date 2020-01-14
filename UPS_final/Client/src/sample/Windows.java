package sample;

import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.GridPane;
import javafx.stage.Stage;

import java.io.PrintWriter;

/**
 * Represents display javafx windows
 */
public class Windows {
    private static final int MATRIX_SIZE = 25;

    /**
     * Popup window, telling some information
     *
     * @param type      type of information
     * @param text      text of information
     */
    public static void popupWindow(String type, String text)
    {
        Alert alert;
        if(type.equals("Information"))
        {
            alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setTitle("Information Dialog");
            alert.setHeaderText("Information");
        }
        else if(type.equals("Warning"))
        {
            alert = new Alert(Alert.AlertType.WARNING);
            alert.setTitle("Warning Dialog");
            alert.setHeaderText("Warning");
        }
        else
        {
            alert = new Alert(Alert.AlertType.ERROR);
            alert.setTitle("Error Dialog");
            alert.setHeaderText("Error");
        }
        alert.setContentText(text);
        alert.show();
    }

    /**
     * Window to send nickname to server
     *
     * @param primaryStage      stage
     * @param output            output stream
     */
    public static void nicknameWindow(Stage primaryStage, PrintWriter output)
    {
        GridPane pane = new GridPane();
        TextField nickname = new TextField();
        Label labelNickname = new Label("Enter your nickname");
        Button button = new Button("Submit");
        button.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                // no nickname entered
                if(nickname.getText().equals(""))
                {
                    Windows.popupWindow("Warning", "Enter nickname");
                }
                else
                {
                    output.println("nickname;" + nickname.getText() + "\0");
                    String msg = "";
                }
            }
        });

        pane.add(labelNickname, 1, 1);
        pane.add(nickname, 1, 2);
        pane.add(button, 4, 3);
        primaryStage.setScene(new Scene(pane, 800, 700));
        primaryStage.show();
    }

    /**
     * Lobby window, where player can enter game queue or exit
     *
     * @param primaryStage      stage
     * @param output            output stream
     * @param rh                receiving thread
     * @param ph                ping thread
     * @param waiting           if player is in queue
     */
    public static void lobbyWindow(Stage primaryStage, PrintWriter output, ReceivingHandle rh, PingHandle ph, boolean waiting)
    {
        GridPane pane = new GridPane();
        Button play;

        if(waiting)
        {
            play = new Button("Waiting for second player");
            play.setDisable(true);
        }
        else
        {
            play = new Button("Play");
            play.setOnAction(new EventHandler<ActionEvent>() {
                @Override
                public void handle(ActionEvent actionEvent) {
                    output.println(":play\0");
                }
            });
        }

        Button exit = new Button("Exit");
        exit.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                output.println(":exit\0");
                ph.stop();
                rh.stop();
                System.exit(0);
            }
        });

        pane.add(exit, 1, 5);
        pane.add(play, 5, 5);
        primaryStage.setScene(new Scene(pane, 800, 700));
        primaryStage.show();
    }

    /**
     * Gam window
     *
     * @param primaryStage          stage
     * @param output                output stream
     * @param matrix                game matrix
     * @param turn                  which player is on turn
     */
    public static void gameWindow(Stage primaryStage, PrintWriter output, Button[][] matrix, String turn)
    {
        GridPane pane;

        pane = new GridPane();
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                final int x = i;
                final int y = j;
                Button b = new Button(matrix[i][j].getText());
                b.setOnAction(new EventHandler<ActionEvent>() {
                    @Override
                    public void handle(ActionEvent actionEvent) {
                        output.println(x + ";" + y + "\0");
                    }
                });
                if (turn.equals("me"))
                {
                    if(b.getText().equals("X") || b.getText().equals("O"))
                    {
                        b.setDisable(true);
                    }
                    else
                    {
                        b.setDisable(false);
                    }
                }
                else
                {
                    b.setDisable(true);
                }
                pane.add(b, i, j);
            }
        }

        Label me = new Label("Me: " + Main.getNickname() + " - " + Main.getMySymbol());
        Label enemy, whosTurn;
        if(Main.getMySymbol().equals("X"))
            enemy = new Label("Enemy: " + Main.getEnemyNickname() + " - O");
        else
            enemy = new Label("Enemy: " + Main.getEnemyNickname() + " - X");

        if (turn.equals("me"))
            whosTurn = new Label("Your turn");
        else
            whosTurn = new Label("Enemy turn");

        pane.add(me, MATRIX_SIZE + 3, 1);
        pane.add(enemy, MATRIX_SIZE + 3, 2);
        pane.add(whosTurn, MATRIX_SIZE + 3, 5);


        primaryStage.setScene(new Scene(pane, 800, 700));
        primaryStage.show();
    }
}
