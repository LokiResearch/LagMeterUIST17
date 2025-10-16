package inria.mjolnir.fancyapp;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import java.util.Random;
import java.util.Vector;

/**
 * Created by Sebastien Poulmane on 04/04/2017.
 */

public class MyGame extends View {

    // Gameplay constants
    private final static int GRAVITY = 1; // Defines how fast the square falls.
    private final static int JUMP_FORCE = 15; // Defines how fast the square jumps.
    private final static float SPACE_BETWEEN_WALL_RATIO = 0.3f; // Defines the space between two walls. It's a ratio of view width.
    private final static float WIDTH_RATIO = 0.1f; // Defines how large is the square and the obstacles. It's a ratio of view width.
    private final static float[] HEIGHT_BOUND_RATIO = {0.1f, 0.6f}; // Defines the min and max height of obstacles. It's a ratio of view height.

    // GFX constants
    private final static int SHADOW_OFFSET = 4;

    private HIDUSBManager lagmeter;

    private Paint paint;
    private Rect rectangleBuffer;
    private Rect rectangleBackground;
    private boolean initialized;

    private Random rng;

    private Point posPlayer;
    private int sizePlayer;
    private Vector<Point> posObstacle;
    private Vector<Point> sizeObstacle;
    private int gameSpeed;
    private int gravityApplied;

    public MyGame(Context context) {
        super(context);
        init();
    }

    public MyGame(Context context, AttributeSet atset) {
        super(context, atset);
        init();
    }

    public MyGame(Context context, AttributeSet atset, int defStyleAttr) {
        super(context, atset, defStyleAttr);
        init();
    }

    public void init() {
        rng = new Random();
        paint = new Paint();
        rectangleBuffer = new Rect();
        rectangleBackground = null;
        initialized = false;
        gravityApplied = GRAVITY;
        posObstacle = new Vector<>();
        sizeObstacle = new Vector<>();
        gameSpeed = 1;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        Log.d("GAME", "Finish to inflat and width is " + getWidth());
    }

    public void setLagmeter (HIDUSBManager lagmeter) {
        this.lagmeter = lagmeter;
    }

    public void jump() {
        gravityApplied = -JUMP_FORCE;
    }

    private void applyPhysics() {
        posPlayer.y += gravityApplied;
        gravityApplied += GRAVITY;
        if (posPlayer.y > getHeight()) {
            posPlayer.y = getHeight();
            gravityApplied = 0;
        }
    }

    private void generateWall() {
        Point pos = new Point((int)(getWidth()*(1.0+WIDTH_RATIO)), getHeight());
        float ratioHeight = HEIGHT_BOUND_RATIO[0] + rng.nextFloat()*HEIGHT_BOUND_RATIO[1];
        Point size = new Point((int)(getWidth()*WIDTH_RATIO/2), (int)(getHeight()*ratioHeight));
        posObstacle.add(pos);
        sizeObstacle.add(size);
    }

    private void moveWalls() {
        if (posObstacle.size() == 0)
            generateWall();
        else {
            for (int i = 0; i < posObstacle.size(); i++)
                posObstacle.elementAt(i).x -= gameSpeed;

            int firstObstaclePosition = posObstacle.firstElement().x;
            int firstObstacleSize = sizeObstacle.firstElement().x;
            int lastObstaclePosition = posObstacle.lastElement().x;
            if ((float)(getWidth() - lastObstaclePosition)/getWidth() >= SPACE_BETWEEN_WALL_RATIO)
                generateWall();
            if (firstObstaclePosition + firstObstacleSize < 0) {
                posObstacle.remove(0);
                sizeObstacle.remove(0);
            }
        }
    }

    private boolean collisionDetection() {
        Point playerBound = new Point(posPlayer.x - sizePlayer, posPlayer.x + sizePlayer);
        Point obstacleBound = new Point();
        for (int i = 0; i < posObstacle.size(); i++) {
            obstacleBound.x = posObstacle.get(i).x - sizeObstacle.get(i).x;
            obstacleBound.y = posObstacle.get(i).x + sizeObstacle.get(i).x;
            if (posPlayer.y >= posObstacle.get(i).y - sizeObstacle.get(i).y
                    && ((obstacleBound.x >= playerBound.x && obstacleBound.x <= playerBound.y)
                        || (playerBound.x >= obstacleBound.x && playerBound.x <= obstacleBound.y)))
                    return true;
        }
        return false;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (!initialized) {
            posPlayer = new Point(getWidth()/3, getHeight());
            sizePlayer = getHeight()/12;
            initialized = true;
        }
        applyPhysics();
        moveWalls();

        if (rectangleBackground == null)
            rectangleBackground = new Rect(0, 0, getWidth(), getHeight());
        paint.setColor(getResources().getColor(R.color.gameBackground));
        canvas.drawRect(rectangleBackground, paint);

        for (int i = 0; i < posObstacle.size(); i++)
            drawGameObject(canvas, getResources().getColor(R.color.gameObstacle), posObstacle.get(i), sizeObstacle.get(i).x, sizeObstacle.get(i).y);

        int playerColor;
        if (collisionDetection())
            playerColor = getResources().getColor(R.color.gameSquareRed);
        else
            playerColor = getResources().getColor(R.color.gameSquare);
        drawGameObject(canvas, playerColor, posPlayer, sizePlayer, sizePlayer*2);

        if (lagmeter != null)
            lagmeter.setTimestamp(TimeTag.DISPLAY, System.currentTimeMillis());
        postInvalidate();
    }

    void drawGameObject(Canvas canvas, int color, Point position, int demidith, int height) {
        paint.setColor(getResources().getColor(R.color.gameShadow));
        rectangleBuffer.set(position.x - demidith, position.y - height, position.x + demidith, position.y);
        rectangleBuffer.offset(SHADOW_OFFSET, SHADOW_OFFSET);
        canvas.drawRect(rectangleBuffer, paint);
        paint.setColor(color);
        rectangleBuffer.offset(-SHADOW_OFFSET, -SHADOW_OFFSET);
        canvas.drawRect(rectangleBuffer, paint);
    }

    public void setGameSpeed(int gameSpeed) {
        this.gameSpeed = gameSpeed;
    }
}
