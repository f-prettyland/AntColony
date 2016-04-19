dat = read.csv("/path/to/AntColony/results/rand.csv")
x <- dat$X0.242000

xfit<-seq(min(x),max(x),length=40) 
yfit<-dnorm(xfit,mean=mean(x),sd=sd(x)) 
yfit <- yfit*diff(h$mids[1:2])*length(x) 
plot(xfit,yfit,type="l", col="blue", lwd=2, xlab="XOR shifted PCG value", ylab="Frequency", 
        main="Histogram of Random Frequency with Normal Curve",)

h<-hist(x, col="red", add=TRUE) 
lines(xfit,yfit,  col="blue", lwd=2)

d <- density(x)
plot(d, main="Kernel Density of XOR shifted PCG value", )
polygon(d, col="red", border="blue")