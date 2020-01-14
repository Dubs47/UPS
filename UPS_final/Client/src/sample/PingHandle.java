package sample;

import java.io.PrintWriter;
import java.util.concurrent.TimeUnit;

/**
 * Represents thread, which sends ping messages to server
 */
public class PingHandle implements Runnable
{
    private volatile boolean exit = false;
    PrintWriter output;

    /**
     *
     * @param output    output stream
     */
    public PingHandle(PrintWriter output)
    {
        this.output = output;
    }

    @Override
    public void run() {
        while (!exit)
        {
            output.println("ping\0");
            try {
                TimeUnit.SECONDS.sleep(5);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * stop thread
     */
    public void stop()
    {
        exit = true;
    }
}
