import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { LandingPageComponent } from './pages/landing-page/landing-page.component';
import { ProfilePageComponent } from './pages/profile-page/profile-page.component';
import { BuyPageComponent } from './pages/buy-page/buy-page.component';
import { SellPageComponent } from './pages/sell-page/sell-page.component';


const routes: Routes = [
  { path: 'home', component: LandingPageComponent },
  { path: 'profile', component: ProfilePageComponent },
  { path: 'buy', component: BuyPageComponent },
  { path: 'sell', component: SellPageComponent },
  { path: '', redirectTo: '/home', pathMatch: 'full' },
  { path: '**', redirectTo: '/home' }
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
