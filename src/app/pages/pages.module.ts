import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LandingPageComponent } from './landing-page/landing-page.component';
import { ProfilePageComponent } from './profile-page/profile-page.component';
import { BuyPageComponent } from './buy-page/buy-page.component';
import { SellPageComponent } from './sell-page/sell-page.component';



@NgModule({
  declarations: [LandingPageComponent, ProfilePageComponent, BuyPageComponent, SellPageComponent],
  imports: [
    CommonModule
  ],
  exports: [
    LandingPageComponent,
    ProfilePageComponent,
    BuyPageComponent,
    SellPageComponent
  ]
})
export class PagesModule { }
