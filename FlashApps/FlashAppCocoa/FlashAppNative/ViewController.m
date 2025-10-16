//
//  ViewController.m
//  FlashAppNative
//
//  Created by Géry Casiez on 26/02/2017.
//  Copyright © 2017 Géry Casiez. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    self.view.backgroundColor = [UIColor blackColor];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    // NSLog(@"Touches Began");
    self.view.backgroundColor = [UIColor whiteColor];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    // NSLog(@"Touches Began");
    self.view.backgroundColor = [UIColor blackColor];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
