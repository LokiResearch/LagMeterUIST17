// Géry Casiez - February 2017

package com.gluonapplication;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.stage.Stage;

public class FlashApp extends Application {
	GraphicsContext gc;
	Canvas canvas;
	boolean white = false;
	String MODE = "PANE";
	Pane p;
	VBox root;
	
	public void start(Stage stage) {
		root = new VBox();
		if (MODE == "PANE") {
			//p = new Pane();
			root.setStyle("-fx-background-color: black;");
			root.setOnTouchPressed(e -> switchColor(true));
			root.setOnMousePressed(e -> switchColor(true));
			root.setOnTouchReleased(e -> switchColor(false));
			root.setOnMouseReleased(e -> switchColor(false));
			//root.getChildren().add(p);
		} else {
			canvas = new Canvas ();
			gc = canvas.getGraphicsContext2D();
			
			canvas.setOnMousePressed(e -> {
				white = true;
				redraw();
			});
			
			canvas.setOnMouseReleased(e -> {
				white = false;
				redraw();
			});
			
			canvas.setOnTouchPressed(e -> {
				white = true;
				redraw();
			});
			
			canvas.setOnTouchReleased(e -> {
				white = false;
				redraw();
			});		
			root.getChildren().add(canvas);
		}
		
		
		
		Scene scene = new Scene(root);
		stage.setTitle("LatencyMeasureJavaFX");
		stage.setScene(scene);
		stage.setFullScreen(true);
		stage.show();
		
		if (MODE == "PANE") {
			//p.setWidth(stage.getWidth());
			//p.setHeight(stage.getHeight());			
		} else {
			canvas.setWidth(stage.getWidth());
			canvas.setHeight(stage.getHeight());
			redraw();
		}
	}
	
    public void switchColor(boolean white) {
        if (white) {
          root.setStyle("-fx-background-color: white;");
        }
        else
          root.setStyle("-fx-background-color: black;");
      }
	
	public void redraw() {
		gc.clearRect(0, 0, canvas.getWidth(), canvas.getHeight());
		gc.setFill(white ? Color.WHITE : Color.BLACK);
		gc.fillRect(0, 0, canvas.getWidth(), canvas.getHeight());
		System.out.println("coucou");
	}
	
	public static void main(String[] args) {
		Application.launch(args);
	}
}
